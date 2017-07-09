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

#include "include/zone.hpp"

Zone::Zone(int id, std::string name, int pin, bool enabled, bool invertLogic) :
BlackGPIO(static_cast<gpioName> (pin), direction::output, 
SecureMode){
    Id(id);
    Name(name);
    Enabled(enabled);
    InvertLogic(invertLogic);
}

void Zone::Id(int id) {
    id_ = id;
}

int Zone::Id() const {
    return id_;
}

bool Zone::TurnOff() {
    return setValue(InvertLogic() ? high : low);
}

bool Zone::TurnOn() {
    return setValue(InvertLogic() ? low : high);
}

void Zone::Enabled(bool enabled) {
    enabled_ = enabled;
}

bool Zone::Enabled() const {
    return enabled_;
}

void Zone::InvertLogic(bool invert) {
    invert_logic_ = invert;
}

bool Zone::InvertLogic() const {
    return invert_logic_; 
}

bool Zone::IsOn() {
    return InvertLogic() ? !isHigh() : isHigh();
}

const std::string Zone::Status(){
    return IsOn() ? std::string("On") : std::string("Off");
}

void Zone::Name(std::string name) {
    name_ = name;
}

std::string Zone::Name() const {
    return name_;
}

Zone::~Zone() {
}
