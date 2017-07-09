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
 * File:   zones.hpp
 * Author: Aaron
 *
 * Created on June 13, 2017, 4:19 PM
 */

#ifndef ZONES_HPP
#define ZONES_HPP

#include <string>

#include <yaml-cpp/yaml.h>
#include <BlackLib/BlackLib.h>
#include <memory>
using namespace BlackLib;

class Zone : private virtual BlackGPIO{
public:
    explicit Zone(int id, std::string name, int pin, bool enabled, bool invertLogic);
    virtual ~Zone();
    
    void Id(int id);
    int Id() const;
    
    /*! @brief Sets a friendly name for this zone.
     *
     * Use this function to set a friendly name.
     * Eg: Flower Beds
     * 
     * @param std::string [name] a string value
     * 
     * @sa Name()
     */
    void Name(std::string name);
    /*! @brief Returns the friendly name for this zone.
     *
     * Use this function to get the friendly name for this zone
     * 
     * @return A string value. eg: Flower Beds
     * @sa Name(std::string)
     */
    std::string Name() const;
    /*! @brief Enables/Disables this zone.
     *
     * Use this function to enable or disable this zone.
     * Will be enabled or disabled in all programs.
     * 
     * @param [in] enabled        set true to enable the zone
     * 
     * @sa Enabled()
     */
    void Enabled(bool enabled);
    /*! @brief Is this zone enabled?
     *
     * Use this function to check if zone is enabled.
     * @return True if zone is enabled
     * 
     * @sa Enabled(bool)
     */
    bool Enabled() const;

    
    /*! @brief Specifies the GPIO logic.
     * 
     * Set to true if pin should be high to enable relay.
     * Set to false if pin should be low to enable relay.
     * 
     * @sa InvertLogic()
     */
    void InvertLogic(bool invert);
    /*! @brief Returns specified GPIO logic.
     * 
     * True if pin should be low to enable relay.
     * False if pin should be high to enable relay.
     * 
     * @sa InvertLogic(bool)
     */
    bool InvertLogic() const;
    
    bool TurnOn();
    bool TurnOff();
    
    /*! @brief Checks value of GPIO pin
     * 
     * Determines result using specified logic.
     * 
     * @return true if on otherwise false.
     *
     * @sa InvertLogic(bool)
     * @sa InvertLogic()
     */
    bool IsOn();
    /*! @brief Checks value of GPIO pin by calling IsOn
     * 
     * Creates a string representation of pin status
     * 
     * @return std::string
     *
     * @sa IsOn()
     */
    const std::string Status();
private:
    int id_; /*!< @brief identification number for this zone */
    std::string name_; /*< @brief a friendly name for this zone*/
    bool enabled_; /*< @brief zone enabled?*/
    bool invert_logic_; /*< @brief use inverted logic?*/

protected:
};

using shared_zone = std::shared_ptr<Zone>;

#endif /* ZONES_HPP */