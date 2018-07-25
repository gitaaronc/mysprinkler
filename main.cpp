/*
 * Copyright (c) 2017, Aaron Coombs. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the mysprinkler Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL AARON COOMBS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "include/main.hpp"
#include "include/shutdown.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cerrno>

#include <BlackLib/BlackLib.h>
#include <unistd.h>

void signal_callback(int signum) {
    utils::Logger::Instance().Debug("Caught signal %d", signum);
    
    StopAllZones();
    
    StartShutdown();
    
    cv_.notify_all();
}

void signal_pipe_callback(int signum) {
    utils::Logger::Instance().Info("Caught and ignored signal %d", signum);
}

void LoadZones(const YAML::Node yNodes) {
    for (auto zone = yNodes.begin(); zone != yNodes.end(); ++zone) {
        YAML::Node details;
        details = zone->second;
        
        shared_zone this_zone = std::make_shared<Zone>(
                zone->first.as<int>(0),
                details["name"].as<std::string>(""),
                details["gpio"].as<int>(0),
                details["enabled"].as<bool>(false),
                details["invert_logic"].as<bool>(true));
        
        this_zone->TurnOff();
        ace::utils::Logger::Instance().Debug("Zone %d is %s!",
                this_zone->Id(), this_zone->Status().c_str());
        
        zones_.push_back(this_zone);
        zones_.sort([](const shared_zone& lhs, const shared_zone& rhs){
            return lhs->Id() < rhs->Id();
        });
    }
}

void LoadPrograms(const YAML::Node yNodes) {
    for (auto it = yNodes.begin(); it != yNodes.end(); ++it) {
        
        shared_program program = std::make_shared<Program>();
        
        program->LoadProgram(it->first.as<int>(0), it->second);
        
        if (!program->Disabled()) {
            QueueProgram(program);
            std::tm tm = *std::localtime(&program->StartTime());
            std::stringstream ss;
            ss << std::put_time(&tm, "%Y/%m/%d %T %Z");
            utils::Logger::Instance().Info("Scheduling program %d for %s...",
                    program->Id(), ss.str().c_str());
        } else {
            utils::Logger::Instance().Info("Program %d set to disabled.",
                    program->Id());
        }
        
        
    }
}

void StopAllZones() {
    utils::Logger::Instance().Info("Stopping all zones.");
    
    for (const auto& zone : zones_) {
        utils::Logger::Instance().Debug("Stopping zone %d", zone->Id());
        
        zone->TurnOff();
        
        ace::utils::Logger::Instance().Debug("Zone %d turned %s!",
                zone->Id(), zone->Status().c_str());
    }
}

void RunZones(const std::list<zone_detail>& list_detail) {
    for (const auto& detail : list_detail) {
        if (ShutdownRequested()) break;
        
        auto zone = std::find_if(zones_.begin(), zones_.end(),
                [detail](const shared_zone & z) {
                    return detail.zone_id == z->Id();
                });
                
        if (zone != zones_.end() && !ShutdownRequested() && (*zone)->Enabled()) {
            utils::Logger::Instance().Info("Watering %s, zone %d for %d minutes",
                    (*zone)->Name().c_str(), detail.zone_id, detail.duration);
            
            (*zone)->TurnOn(); // turn on the zone
            
            ace::utils::Logger::Instance().Debug("Zone %d turned %s!",
                    (*zone)->Id(), (*zone)->Status().c_str());
            
            std::unique_lock<std::mutex>lk(zone_mutex_);
            
            cv_.wait_until(lk, std::chrono::system_clock::now() +
                    std::chrono::minutes(detail.duration));
            
            (*zone)->TurnOff(); // turn of the zone
            
            ace::utils::Logger::Instance().Debug("Zone %d turned %s!",
                    (*zone)->Id(), (*zone)->Status().c_str());
        }
    }
}

/**
 * QueueProgram
 * @param program
 */
void QueueProgram(const shared_program& program) {
    if (!programs_.empty()) {
        // check for duplicate
        if (std::find_if(programs_.begin(), programs_.end(), [program](
                const shared_program & right) {
                return program->Id() == right->Id();
            }) != programs_.end()) {
        utils::Logger::Instance().Info("Rejecting duplicate program ID: %d",
                program->Id());
    } else {
            // insert program in order scheduled by date/time
            // if date and time are duplicates, this scheduled after existing
            // TODO: warn of conflicting schedule? 
            auto lower = std::lower_bound(programs_.begin(), programs_.end(),
                    program, [](const shared_program& left, const shared_program & right) {
                        return left->StartTime() < right->StartTime();
                    });
            programs_.insert(lower, program);
        }
    } else {
        // deque is empty add program
        programs_.push_back(program);
    }
}

bool MainLoop() {
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&tt);
    std::stringstream ss;

    while (!ShutdownRequested()) {
        if (!programs_.empty()) {
            // get a program from the front of deque
            shared_program program = programs_.front();
            programs_.pop_front(); // remove program from the front of deque

            tm = *std::localtime(&program->StartTime());

            ss.str(std::string());
            ss << std::put_time(&tm, "%T %Z on %Y/%m/%d");
            utils::Logger::Instance().Info("Program %i scheduled to run at %s",
                    program->Id(), ss.str().c_str());

            std::unique_lock<std::mutex>lk(program_mutex_);
            if (cv_.wait_until(lk, std::chrono::system_clock::from_time_t(
                    program->StartTime())) == std::cv_status::timeout) {
                RunZones(program->ZoneDetail()); // run the program zone
                program->NextStartTime(); // set the next starting time
                tm = *std::localtime(&program->StartTime());

                ss.str(std::string());
                ss << std::put_time(&tm, "%Y/%m/%d at %T %Z");
                utils::Logger::Instance().Info("Program %i completed and will run"
                        " again on %s", program->Id(), ss.str().c_str());
            }
            QueueProgram(program); // add the updated program to the deque
        } else {
            utils::Logger::Instance().Info("Nothing to do. Exiting...");
            StartShutdown();
        }
    }
    return true;
}
bool AppInit(int argc, char* argv[]){
    bool fRet = false;
    if (argc < 2) {
        std::cout << "No arguments found on the command line.\n";
        std::cout << "Expecting configuration file on the command line.\n";
        std::cout << "eg: mysprinkler /etc/mysprinkler.yaml\n";
        return 0;
    }
    std::string config_file_;
    config_file_.assign(argv[1]);
    YAML::Node yConfig;
    try {
        yConfig = YAML::LoadFile(config_file_);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
        return 0;
    }

    // get and set logging mode, default is NONE, no logging
    std::string logging_mode = yConfig["logging_mode"].as<std::string>("NONE");
    std::transform(logging_mode.begin(), logging_mode.end(),
            logging_mode.begin(), ::tolower);
    if (logging_mode.compare("none") == 0) {
        ace::utils::Logger::Instance().SetLoggingMode(
                ace::utils::Logger::NONE);
    } else if (logging_mode.compare("info") == 0) {
        ace::utils::Logger::Instance().SetLoggingMode(
                ace::utils::Logger::INFO);
    } else if (logging_mode.compare("warning") == 0) {
        ace::utils::Logger::Instance().SetLoggingMode(
                ace::utils::Logger::WARNING);
    } else if (logging_mode.compare("debug") == 0) {
        ace::utils::Logger::Instance().SetLoggingMode(
                ace::utils::Logger::DEBUG);
    } else if (logging_mode.compare("trace") == 0) {
        ace::utils::Logger::Instance().SetLoggingMode(
                ace::utils::Logger::TRACE);
    } else if (logging_mode.compare("verbose") == 0) {
        ace::utils::Logger::Instance().SetLoggingMode(
                ace::utils::Logger::VERBOSE);
    }

    if (yConfig["daemon"].as<bool>(false)){
        if (daemon(1,0)){
            fprintf(stderr, "Error: daemon() failed: %s\n", strerror(errno));
            return errno;
        }
    }
    
    LoadZones(yConfig["ZONES"]);

    LoadPrograms(yConfig["PROGRAMS"]);

    fRet = MainLoop();

    std::ofstream ofs(config_file_);
    ofs << yConfig;
    ofs.close();
    
    ace::utils::Logger::Instance().Info("mysprinkler exited cleanly.");
    return fRet;
}

int main(int argc, char** argv) {
    std::signal(SIGTERM, signal_callback);
    std::signal(SIGINT, signal_callback);
    std::signal(SIGPIPE, signal_pipe_callback);

    return (AppInit(argc, argv)) ? EXIT_SUCCESS : EXIT_FAILURE;
}

