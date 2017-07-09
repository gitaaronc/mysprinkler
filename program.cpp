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

#include "include/program.hpp"

#include <string>
#include <chrono>
#include <thread>
#include <algorithm>

Program::Program() : id_(-1), hour_(0), minute_(0), interval_(1),
rain_delay_(false) {

}

Program::~Program() {

}

bool Program::Disabled() {
    return disabled_;
}

void Program::Disabled(bool disabled) {
    disabled_ = disabled;
}

const int Program::Id() {
    return id_;
}

void Program::LoadWeekdays(YAML::Node weekdays) {
    for (int c = 0; c < weekdays.size(); c++) {
        std::string day = weekdays[c].as<std::string>("NAN");
        std::transform(day.begin(), day.end(), day.begin(), ::tolower);
        if (day.compare(0, 3, "sun") == 0) {
            weekdays_.push_back(0);
        } else if (day.compare(0, 3, "mon") == 0) {
            weekdays_.push_back(1);
        } else if (day.compare(0, 3, "tue") == 0) {
            weekdays_.push_back(2);
        } else if (day.compare(0, 3, "wed") == 0) {
            weekdays_.push_back(3);
        } else if (day.compare(0, 3, "thu") == 0) {
            weekdays_.push_back(4);
        } else if (day.compare(0, 3, "fri") == 0) {
            weekdays_.push_back(5);
        } else if (day.compare(0, 3, "sat") == 0) {
            weekdays_.push_back(6);
        }
    }
    if (!weekdays_.empty())
        std::sort(weekdays_.begin(), weekdays_.end());
}

void Program::LoadProgram(int id, YAML::Node node) {
    id_ = id;
    hour_ = node["hour"].as<int>(0);
    minute_ = node["minute"].as<int>(0);
    SetMode(node["mode"].as<std::string>("interval"));
    interval_ = node["interval"].as<bool>(false);
    rain_delay_ = node["rain_delay"].as<bool>(false);
    disabled_ = node["disabled"].as<bool>(false);
    LoadWeekdays(node["weekdays"]);

    YAML::Node zNode = node["zone_detail"];
    for (auto it = zNode.begin(); it != zNode.end(); ++it) {
        zone_details_.push_back(zone_detail(
                it->first.as<int>(-1),
                it->second["duration"].as<int>(-1)));
    }
    zone_details_.sort([](const zone_detail& lhs, const zone_detail& rhs){
        return lhs.zone_id < rhs.zone_id;
    });
    NextStartTime();
}

const std::time_t& Program::StartTime() {
    return next_runtime_;
}

void Program::NextStartTime() {
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm ttm = *std::localtime(&tt);

    ttm.tm_hour = hour_;
    ttm.tm_min = minute_;
    ttm.tm_sec = 0;
    next_runtime_ = std::mktime(&ttm);

    SetDay(ttm);

    next_runtime_ = std::mktime(&ttm);
}

void Program::SetMode(std::string mode) {
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    if (mode.compare("even_only") == 0) {
        mode_ = MODE::even_only;
    } else if (mode.compare("odd_only") == 0) {
        mode_ = MODE::odd_only;
    } else if (mode.compare("weekdays") == 0) {
        mode_ = MODE::weekdays;
    } else if (mode.compare("interval") == 0) {
        mode_ = MODE::interval;
    }
}

void Program::SetDay(std::tm& tm) {
    bool isOdd = false;
    bool isLeapYear = false;

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_now = *std::localtime(&tt);

    std::time_t now = std::mktime(&tm_now);
    if (std::difftime(next_runtime_, now) <= 0) {
        tm.tm_mday++; // increase the day by 1
        mktime(&tm); // normalize tm struct
    }

    isOdd = tm.tm_mday % 2 != 0; // is this an odd number day?
    isLeapYear = tm.tm_year % 4 == 0;

    switch (mode_) {
        case MODE::even_only:
        {
            tm.tm_mday += isOdd ? 1 : 0;
        }
            break;
        case MODE::odd_only:
        {
            tm.tm_mday += isOdd ? 0 : 1;
        }
            break;
        case MODE::interval:
        {
            tm.tm_mday += interval_ > 0 ? interval_ : 0;
        }
            break;
        case MODE::weekdays:
        {
            if (weekdays_.empty()) { 
                Disabled(true); // disable misconfigured program
                break; // no need to go any further
            }
            while (true) {// iterate vector<int> and compare with day (0..6)
                auto weekday = std::find_if(weekdays_.begin(), weekdays_.end(),
                        [&tm](const int rhs) {
                            return tm.tm_wday == rhs; // this day is in vector
                        });
                if (weekday == weekdays_.end()) { // this day not in vector
                    tm.tm_mday++; // increase this day
                    mktime(&tm); // normalize
                } else {
                    break;
                }
            }
        }
            break;
    }

    mktime(&tm); // normalize tm struct
    // skip the last odd day of the month
    if (tm.tm_mday == 31 || (tm.tm_mday == 29 && tm.tm_mon == 2 && isLeapYear)) {
        tm.tm_mday++;
    }

}

std::list<zone_detail> Program::ZoneDetail() {
    return zone_details_;
}
