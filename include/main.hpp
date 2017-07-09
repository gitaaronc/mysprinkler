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
 * File:   main.hpp
 * Author: Aaron
 *
 * Created on June 18, 2017, 6:57 AM
 */

#ifndef MAIN_HPP
#define MAIN_HPP

#include "Logger.h"
#include "zone.hpp"
#include "program.hpp"

#include <yaml-cpp/yaml.h>

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

using namespace ace;

bool is_running_ = true;
std::deque<shared_program> programs_; 
std::list<shared_zone> zones_; 
std::condition_variable cv_;
std::mutex program_mutex_;
std::mutex zone_mutex_;
bool is_daemon_;

void LoadPrograms(const YAML::Node yNodes);
void LoadZones(const YAML::Node yNodes);
void QueueProgram(const shared_program& program);
void RunZones(const std::list<zone_detail>& list_detail);
void StopAllZones();
void MainLoop();

#endif /* MAIN_HPP */

