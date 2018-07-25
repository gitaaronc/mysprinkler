/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "include/shutdown.hpp"

#include <atomic>

static std::atomic<bool> fRequestShutdown(false);

void StartShutdown(){
    fRequestShutdown = true;
}

void AbortShutdown(){
    fRequestShutdown = false;
}

bool ShutdownRequested(){
    return fRequestShutdown;
}