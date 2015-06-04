//----------------------------------------------------------------------------------
// File:        NvGamepad/NvGamepadXInput.cpp
// SDK Version: v2.0 
// Email:       gameworks@nvidia.com
// Site:        http://developer.nvidia.com/
//
// Copyright (c) 2014, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------------
#ifdef WIN32

#include "NvGamepad/NvGamepadXInput.h"
#include <Windows.h>
#include <xinput.h>
#include <math.h>

NvGamepadXInput::NvGamepadXInput() {
    mStates = new State[MAX_GAMEPADS];
    mXStates = new XINPUT_STATE[MAX_GAMEPADS];

    memset(mStates, 0, MAX_GAMEPADS * sizeof(State));
    memset(mXStates, 0, MAX_GAMEPADS * sizeof(mXStates));

    int32_t i;
    for (i = 0; i < MAX_GAMEPADS; i++) {
        XInputGetState(i, mXStates+i);
        updateFromXState(mStates[i], mXStates[i]);
    }
}

NvGamepadXInput::~NvGamepadXInput() {
    delete[] mStates;
    delete[] mXStates;
}

bool NvGamepadXInput::getState(int32_t padID, State& state) {
    if (ERROR_SUCCESS != XInputGetState(padID, mXStates+padID))
        return false;

    if (mXStates[padID].dwPacketNumber != mStates[padID].mTimestamp)
        updateFromXState(mStates[padID], mXStates[padID]);

    state = mStates[padID];

    return true;
}

uint32_t NvGamepadXInput::pollGamepads() {
    int32_t changedMask = 0;
    int32_t i;
    for (i = 0; i < MAX_GAMEPADS; i++) {
        uint32_t timestamp = mXStates[i].dwPacketNumber;
        if (ERROR_SUCCESS == XInputGetState(i, mXStates+i)) {
            if (timestamp != mStates[i].mTimestamp)
                changedMask |= (1<<i);
        }
    }

    return changedMask;
}

void NvGamepadXInput::setMaxGamepadCount(int32_t max) {
    /* */
}

int32_t NvGamepadXInput::getMaxGamepadCount() {
    return MAX_GAMEPADS;
}

void NvGamepadXInput::updateFromXState(State& dest, const XINPUT_STATE& src) {
    float lx = src.Gamepad.sThumbLX;
    float ly = src.Gamepad.sThumbLY;
    float lMag = sqrtf(lx*lx + ly*ly);

    if (lMag > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) {
        lx /= lMag;
        ly /= lMag;

        lMag = (lMag - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)/ float(32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

        lx *= lMag;
        ly *= lMag;
    }
    else {
        lx = ly = 0.0f;
    }

    float rx = src.Gamepad.sThumbRX;
    float ry = src.Gamepad.sThumbRY;
    float rMag = sqrtf(rx*rx + ry*ry);

    if (rMag > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) {
        rx /= rMag;
        ry /= rMag;

        rMag = (rMag - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)/ float(32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

        rx *= rMag;
        ry *= rMag;
    }
    else {
        rx = ry = 0.0f;
    }

    float lt = 0.0f;
    if ( src.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
        lt = float(src.Gamepad.bLeftTrigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (255 -XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

    float rt = 0.0f;
    if ( src.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
        rt = float(src.Gamepad.bRightTrigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (255 -XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

    dest.mTimestamp = src.dwPacketNumber;
    dest.mButtons = src.Gamepad.wButtons;
    dest.mThumbLX = lx;
    dest.mThumbLY = -ly; // flip to match Android
    dest.mThumbRX = rx;
    dest.mThumbRY = -ry; // flip to match Android
    dest.mLeftTrigger = lt;
    dest.mRightTrigger = rt;
}

#endif // WIN32
