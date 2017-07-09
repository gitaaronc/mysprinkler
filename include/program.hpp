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

/* 
 * File:   program.hpp
 * Author: Aaron
 *
 * Created on June 15, 2017, 8:33 PM
 */

#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <list>
#include <ctime>

#include <yaml-cpp/yaml.h>

// holds custom details of a given zone fur use by the Program object
struct zone_detail {
    zone_detail(int zone_id, int duration) : zone_id(zone_id), duration(duration)
    {}
    int zone_id;
    int duration;
};

// program mode
enum MODE {
    even_only, odd_only, weekdays, interval
};

class Program {
public:
    Program();
    ~Program();
    const int Id(); // returns the program id
    void LoadProgram(int id, YAML::Node node); // Loads the program from config
    const std::time_t& StartTime(); // return the set Start Time
    void NextStartTime(); // sets the next starting time/day
    std::list<zone_detail> ZoneDetail(); // returns a list of zones to run
    bool Disabled();
    void Disabled(bool disabled);
private:
    void LoadWeekdays(YAML::Node weekdays);
    void SetMode(std::string mode); // set the mode of the program
    void SetDay(std::tm & tm); // helper to set the next runtime (day))
    int id_; // Program ID, user defined.
    int hour_; // hour which to start the program
    int minute_; // minutes after the hour to start the program
    MODE mode_; // watering mode - even_only, odd_only, interval or weekdays
    int interval_; // number of days between watering
    bool rain_delay_; // are we delaying this program when it rains
    std::vector<int> weekdays_; // list of weekdays the program will run
    bool disabled_;
    // TODO replace std::list<zone_detail> with a map
    std::list<zone_detail> zone_details_; // list of zones used in this program
    std::time_t next_runtime_;
protected:
};

using shared_program = std::shared_ptr<Program>;

#endif /* PROGRAM_HPP */

