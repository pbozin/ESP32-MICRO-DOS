#include "microdos_api.h"

#define CLAMP(x, l, h) ((x) < (l) ? (l) : ((x) > (h) ? (h) : (x)))

static const uint8_t scaleMinor[5][8] ALIGNED = {
    {60, 62, 63, 65, 67, 68, 70, 0}, {57, 59, 60, 62, 64, 65, 67, 0},
    {59, 61, 62, 64, 66, 67, 69, 0}, {55, 57, 58, 60, 62, 63, 65, 0},
    {62, 64, 65, 67, 69, 70, 72, 0}
};
static const uint8_t scaleMajor[5][8] ALIGNED = {
    {60, 62, 64, 65, 67, 69, 71, 0}, {62, 64, 66, 67, 69, 71, 73, 0},
    {65, 67, 69, 70, 72, 74, 76, 0}, {57, 59, 61, 62, 64, 66, 68, 0},
    {67, 69, 71, 72, 74, 76, 78, 0}
};
static const uint8_t scaleDorian[5][8] ALIGNED = {
    {60, 62, 63, 65, 67, 69, 70, 0}, {62, 64, 65, 67, 69, 71, 72, 0},
    {58, 60, 61, 63, 65, 67, 68, 0}, {55, 57, 58, 60, 62, 64, 65, 0},
    {64, 66, 67, 69, 71, 73, 74, 0}
};
static const uint8_t scaleAbstract[5][8] ALIGNED = {
    {58, 59, 62, 63, 66, 67, 70, 0}, {56, 57, 60, 61, 64, 65, 68, 0},
    {54, 55, 58, 59, 63, 64, 66, 0}, {52, 53, 56, 57, 61, 62, 64, 0},
    {50, 51, 54, 55, 58, 59, 62, 0}
};
static const uint8_t scaleGoa[5][8] ALIGNED = {
    {40, 41, 44, 45, 47, 48, 50, 0}, {45, 47, 48, 50, 52, 53, 56, 0},
    {48, 50, 52, 53, 56, 57, 59, 0}, {40, 44, 47, 48, 50, 52, 53, 0},
    {41, 45, 47, 48, 50, 52, 55, 0}
};
static const uint8_t scaleAeolian[5][8] ALIGNED = {
    {36, 38, 39, 41, 43, 44, 46, 0}, {44, 46, 47, 49, 51, 52, 54, 0},
    {41, 43, 44, 46, 48, 49, 51, 0}, {36, 38, 39, 41, 43, 44, 46, 0},
    {39, 41, 42, 44, 46, 47, 49, 0} 
};

static uint8_t markovMatrix[5][5] ALIGNED = {
    {40, 30, 10, 10, 10}, {20, 30, 30, 10, 10}, 
    {10, 20, 40, 20, 10}, {30, 10, 20, 30, 10},
    {10, 20, 30, 20, 20} 
};

#define SYNTH_START 1
#define SYNTH_END   4
#define BASS_START  5
#define BASS_END    8
#define DRUM_START  9
#define DRUM_END    12
#define HAT_START   13
#define HAT_END     16

static uint32_t tickCount           = 0;
static uint32_t longBarCounter      = 0;
static uint32_t styleExpirationBar  = 90;
static uint32_t stepIntervalMs      = 212;

static uint16_t currentBpm  ALIGNED = 94;

static uint8_t goa_engine_state[12] ALIGNED = {
    0,  // [0]  goaNoteDurationCounter
    0,  // [1]  currentChordIndex
    3,  // [2]  goaMotiveChance
    2,  // [3]  goaMotiveDuration
    10, // [4]  changeRate
    20, // [5]  globalDrumDensity
    0,  // [6]  currentArtistStyle
    50, // [7]  dynamicArpChance
    4,  // [8]  dynamicHatSkip
    30, // [9]  breakrollBase
    1,  // [10] melodyDirection (Packed securely as an int8_t)
    0   // [11] HARDWARE ALIGNMENT PADDING CAP (Forces divisible-by-4 payload)
};

#define goaNoteDurationCounter (goa_engine_state[0])
#define currentChordIndex      (goa_engine_state[1])
#define goaMotiveChance        (goa_engine_state[2])
#define goaMotiveDuration      (goa_engine_state[3])
#define changeRate             (goa_engine_state[4])
#define globalDrumDensity      (goa_engine_state[5])
#define currentArtistStyle     (goa_engine_state[6])
#define dynamicArpChance       (goa_engine_state[7])
#define dynamicHatSkip         (goa_engine_state[8])
#define breakrollBase          (goa_engine_state[9])
#define melodyDirection        ((int8_t&)goa_engine_state[10])

static uint8_t engine_state_registers[4] ALIGNED = {3, 0, 0, 0};
#define currentTimeSignature   (engine_state_registers[0])
#define currentMelodyStyle     (engine_state_registers[1])
#define lastScalePositionIndex (engine_state_registers[2])
#define activeMelodyBaseNote   (engine_state_registers[3])

static uint8_t dynamic_voice_channels[4] ALIGNED = {1, 5, 9, 13};
#define activeSynthChannel (dynamic_voice_channels[0])
#define activeBassChannel  (dynamic_voice_channels[1])
#define activeDrumChannel  (dynamic_voice_channels[2])
#define activeHatChannel   (dynamic_voice_channels[3])

static uint8_t initial_mix_config[4] ALIGNED = {30, 15, 32, 0};
#define glitchPercussionChance (initial_mix_config[0])
#define ghostKickChance        (initial_mix_config[1])
#define activePanIntensity     (initial_mix_config[2])
#define globalKeyTransposition (initial_mix_config[3])

static uint8_t lastArpNotes[4] ALIGNED;
static uint8_t lastDroneNotes[4] ALIGNED;
static bool systemMuteArray[17] ALIGNED  = {false};
static bool bassMelodyInherit ALIGNED    = false;

static uint8_t mdb_alignment_shield[4] ALIGNED = {0, 0, 0, 0};

static const uint8_t signature_multipliers[4] ALIGNED = {85, 0, 51, 0};

ALWAYS INLINE void midiMsg(uint8_t cmd, uint8_t d1, uint8_t d2) {
    uint8_t data_packet[] = { cmd, d1, d2 };
    serialWrite(data_packet, 3);
}

ALWAYS INLINE void displayValue(uint8_t simpleDisplayValue, uint8_t row) {
    char buf[12];
    itoa(simpleDisplayValue, buf);
    printAt(30, row, buf);
}


ALWAYS INLINE void selectElectribePattern(uint16_t patternIndex) {
    if (patternIndex < 1 || patternIndex > 250) return;
    uint8_t cc32_lsb = (patternIndex <= 127) ? 0 : 1;
    uint8_t program  = (patternIndex <= 127) ? (patternIndex - 1) : (patternIndex - 128);

    midiMsg(0xB0, 0, 0);
    midiMsg(0xB0, 32, cc32_lsb);
    midiMsg(0xC0, program, 0);

    displayValue(patternIndex, 2);
    delay(40);
}

ALWAYS INLINE void silenceAllChannels() {
    for (uint8_t ch = 1; ch <= 16; ch++) midiMsg(0xB0 | (ch - 1), 123, 0);
}

ALWAYS INLINE void randomizeElectribePattern() {
    silenceAllChannels();
    selectElectribePattern(random(1, 5));
}

ALWAYS INLINE void changeMarkovMatrix(uint8_t mode) {
    if (mode == 0 || mode == 1 || mode == 5) {
        markovMatrix[0][0] = 10; markovMatrix[0][1] = 80; markovMatrix[0][2] = 10; markovMatrix[0][3] = 0;  markovMatrix[0][4] = 0;
        markovMatrix[1][0] = 0;  markovMatrix[1][1] = 10; markovMatrix[1][2] = 80; markovMatrix[1][3] = 10; markovMatrix[1][4] = 0;
        markovMatrix[2][0] = 0;  markovMatrix[2][1] = 0;  markovMatrix[2][2] = 10; markovMatrix[2][3] = 80; markovMatrix[2][4] = 10;
        markovMatrix[3][0] = 10; markovMatrix[3][1] = 0;  markovMatrix[3][2] = 0;  markovMatrix[3][3] = 10; markovMatrix[3][4] = 80;
        markovMatrix[4][0] = 80; markovMatrix[4][1] = 0;  markovMatrix[4][2] = 10; markovMatrix[4][3] = 0;  markovMatrix[4][4] = 10;
    } else {
        uint8_t grid = currentTimeSignature > 5 ? 5 : currentTimeSignature;
        for (int i = 0; i < grid; i++) {
            uint32_t total = 0; 
            uint32_t weights[5];
            
            for (int j = 0; j < grid; j++) { 
                weights[j] = random(10, 100); 
                total += weights[j]; 
            }
            
            if (total == 0) total = 1;
            
            uint32_t reciprocalTotal = 65536U / total;
            
            int sum = 0;
            for (int j = 0; j < grid - 1; j++) {
                uint32_t scaledValue = (weights[j] * 100 * reciprocalTotal) >> 16;
                markovMatrix[i][j] = (uint8_t)scaledValue;
                sum += markovMatrix[i][j];
            }
            
            markovMatrix[i][grid - 1] = 100 - sum;
        }   
    }       
}

ALWAYS INLINE void randomSynthChannel() {
    if (activeSynthChannel >= SYNTH_START && activeSynthChannel <= SYNTH_END) {
        midiMsg(0xB0 | (activeSynthChannel - 1), 123, 0);
    }
    activeSynthChannel = random(SYNTH_START, SYNTH_END + 1);
    displayValue(activeSynthChannel, 4);
}

ALWAYS INLINE void randomHatChannel() {
    if (activeHatChannel >= HAT_START && activeHatChannel <= HAT_END) {
        midiMsg(0xB0 | (activeHatChannel - 1), 123, 0);
    }
    activeHatChannel = random(HAT_START, HAT_END + 1);
    displayValue(activeHatChannel, 7);
}

ALWAYS INLINE void randomDrumChannel() {
    if (activeDrumChannel >= DRUM_START && activeDrumChannel <= DRUM_END) {
        midiMsg(0xB0 | (activeDrumChannel - 1), 123, 0);
    }
    activeDrumChannel = random(DRUM_START, DRUM_END + 1);
    displayValue(activeDrumChannel, 6);
}

ALWAYS INLINE void randomBassChannel() {
    if (activeBassChannel >= BASS_START && activeBassChannel <= BASS_END) {
        midiMsg(0xB0 | (activeBassChannel - 1), 123, 0);
    }

    activeBassChannel = random(BASS_START, BASS_END + 1);

    displayValue(activeBassChannel, 5);

#ifdef SYNTH_XFM
    uint8_t safeBase = (BASS_START > 0) ? (BASS_START - 1) : 0;

    uint8_t soundA = random(0, 16);
    uint8_t soundB = random(0, 16);
    changeXFMSynths(soundA, soundB, safeBase);
#endif
}

ALWAYS INLINE void transitionToNextArtist() {
    silenceAllChannels();
    randomizeElectribePattern();
    uint8_t nextStyle = currentArtistStyle + random(1, 5);
    while (nextStyle >= 5) {
        nextStyle -= 5;
    }
    currentArtistStyle = nextStyle;

    uint8_t currentMarkov = random(0, 3);
    changeMarkovMatrix(currentMarkov);
    silenceAllChannels();

    switch (currentArtistStyle) {
        case 0:
            currentTimeSignature = random(3, 6); currentBpm = random(90, 100);
            dynamicArpChance = 40; dynamicHatSkip = 6; globalDrumDensity = 20;
            breakrollBase = random(15, 30); styleExpirationBar = random(300, 400);
            changeRate = random(5, 10);
            break;
        case 1:
            currentTimeSignature = random(3, 6); currentBpm = random(78, 80);
            dynamicArpChance = 65; dynamicHatSkip = 3; globalDrumDensity = 30;
            breakrollBase = random(10, 25); styleExpirationBar = random(300, 400);
            changeRate = random(10, 20);
            break;
        case 2:
            currentTimeSignature = random(3, 6); currentBpm = random(80, 100);
            dynamicArpChance = 55; dynamicHatSkip = 2; globalDrumDensity = 35;
            breakrollBase = random(10, 25); styleExpirationBar = random(300, 400);
            changeRate = random(5, 10);
            break;
        case 3:
            currentTimeSignature = random(3, 6); currentBpm = random(80, 90);
            dynamicArpChance = 35; dynamicHatSkip = 8; globalDrumDensity = 15;
            breakrollBase = random(20, 30); styleExpirationBar = random(300, 400);
            changeRate = random(10, 20);
            break;
        default:
        case 4:
            currentTimeSignature = 5; currentBpm = random(60, 68);
            dynamicArpChance = 20; dynamicHatSkip = 8; globalDrumDensity = 0;
            breakrollBase = 0; styleExpirationBar = random(300, 400);
            changeRate = 4; changeMarkovMatrix(5);
            break;
    }

    displayValue(currentArtistStyle, 3);
    displayValue(currentBpm, 1);
    
    randomDrumChannel(); randomHatChannel(); randomSynthChannel(); randomBassChannel();
    for (int i = 0; i <= 16; i++) systemMuteArray[i] = false;

    uint32_t msPerBar = (60000 * 179) >> 14;
    
    if (currentTimeSignature == 3) {
        stepIntervalMs = (msPerBar * signature_multipliers[0]) >> 8;
    } else if (currentTimeSignature == 4) {
        stepIntervalMs = msPerBar >> 2;
    } else if (currentTimeSignature == 5) {
        stepIntervalMs = (msPerBar * signature_multipliers[2]) >> 8;
    } else {
        stepIntervalMs = msPerBar >> 2;
    }

}

void processMicroEvolution() {
    if (longBarCounter >= styleExpirationBar) {
        longBarCounter = 0;
        transitionToNextArtist();
        return;
    }

    if (random(0, 100) < changeRate) { randomSynthChannel(); return; }
    if (random(0, 100) < changeRate) { randomHatChannel(); return; }
    if (random(0, 100) < changeRate) { randomBassChannel(); return; }
    if (random(0, 100) < changeRate) { randomDrumChannel(); return; }

    if (longBarCounter % 4 == 0) {
        uint8_t breakRoll = random(0, 100);

        if (breakRoll < breakrollBase) {
            systemMuteArray[activeDrumChannel] = true;
            systemMuteArray[activeSynthChannel] = false;
        }
        else if (breakRoll < 60) {
            systemMuteArray[activeDrumChannel] = false;
            systemMuteArray[activeSynthChannel] = true;
        }
        else if (breakRoll < 90) {
            systemMuteArray[activeHatChannel] = true;
        }
        else {
            systemMuteArray[activeDrumChannel] = false;
            systemMuteArray[activeHatChannel] = false;
            systemMuteArray[activeSynthChannel] = false;
        }
    }
    displayValue(longBarCounter, 8);
}

ALWAYS INLINE void processPercussionEngine(uint8_t stepBase, uint8_t beatsPerBar) {
    if (!systemMuteArray[activeDrumChannel]) {
        if (stepBase == 0) {
            midiMsg(0xB0 | (activeDrumChannel - 1), 10, 64);
            midiMsg(0x90 | (activeDrumChannel - 1), 36, 105);
	} else if (stepBase == (beatsPerBar >> 1) && currentTimeSignature > 3) {
            if (random(0, 100) < 50) {
                midiMsg(0xB0 | (activeDrumChannel - 1), 10, 64);
                midiMsg(0x90 | (activeDrumChannel - 1), 36, 105);
            }
        } else if (stepBase == (beatsPerBar - 1) && (random(0, 100) < ghostKickChance)) {
            midiMsg(0xB0 | (activeDrumChannel - 1), 10, 64);
            midiMsg(0x90 | (activeDrumChannel - 1), 36, random(45, 65));
        }
        if (stepBase == (beatsPerBar - 1) && random(0, 100) < globalDrumDensity) {
            midiMsg(0xB0 | (activeDrumChannel - 1), 10, 64);
            midiMsg(0x90 | (activeDrumChannel - 1), 41, random(45, 68));
        }
    }
    if (!systemMuteArray[activeHatChannel]) {
        uint8_t hatRoll = random(0, 100);
        uint8_t targetTriggerThreshold = 30;
        if (stepBase == 0) {
            targetTriggerThreshold = 15;
        } else if (stepBase == 1 || stepBase == (beatsPerBar - 1)) {
            targetTriggerThreshold = 65;
        } else if (stepBase % 2 == 0) {
            targetTriggerThreshold = 45;
        }
        if (currentArtistStyle == 0 || currentArtistStyle == 3) {
            targetTriggerThreshold = targetTriggerThreshold >> 1;
        }
        if (hatRoll < targetTriggerThreshold) {
            uint8_t softVelocity = 28 + random(0, 20) + (stepBase * 2);
            uint8_t targetHatNote = 42;
            if (softVelocity < 38) {
                targetHatNote = 44;
            } else if (currentArtistStyle == 2 && hatRoll < 5) {
                targetHatNote = 46;
            }
            uint8_t hatPan = 64 + random(-(activePanIntensity / 2), (activePanIntensity / 2) + 1);
            midiMsg(0xB0 | (activeHatChannel - 1), 10, CLAMP(hatPan, 20, 108));
            uint8_t dynamicDecayValue = 20 + (softVelocity / 2) + random(0, 10);
            midiMsg(0xB0 | (activeHatChannel - 1), 72, CLAMP(dynamicDecayValue, 15, 80));
            midiMsg(0x90 | (activeHatChannel - 1), targetHatNote, softVelocity);
        }
    }
    if (stepBase == (beatsPerBar - 1)) {
        longBarCounter++;
    }
}

ALWAYS INLINE void processBassEngine(uint8_t stepBase) {
    if (systemMuteArray[activeBassChannel]) return;
    uint8_t matrixIdx = activeBassChannel - BASS_START;
    if (matrixIdx >= 4) return;

    bool shouldCutNote = false;
    if (currentTimeSignature == 3 && stepBase == 2) shouldCutNote = true;
    else if (stepBase == (currentTimeSignature - 1)) shouldCutNote = true;

    if (lastDroneNotes[matrixIdx] > 0 && shouldCutNote) {
        midiMsg(0x80 | (activeBassChannel - 1), lastDroneNotes[matrixIdx], 0);
        lastDroneNotes[matrixIdx] = 0;
    }

    bool dynamicBassTrigger = false;
    if (stepBase == 0)                                            dynamicBassTrigger = (random(0, 100) < 85);
    else if (currentTimeSignature == 3 && stepBase == 1)          dynamicBassTrigger = (random(0, 100) < 40);
    else if (currentTimeSignature == 5 && (stepBase == 2 || stepBase == 3)) dynamicBassTrigger = (random(0, 100) < 50);
    else if (currentTimeSignature == 4 && stepBase == 2) dynamicBassTrigger = (random(0, 100) < 30);
    if (dynamicBassTrigger) {
        if (lastDroneNotes[matrixIdx] > 0) {
            midiMsg(0x80 | (activeBassChannel - 1), lastDroneNotes[matrixIdx], 0);
        }
        uint8_t targetBassNote = 0;
        if (bassMelodyInherit && activeMelodyBaseNote > 0) {
            uint8_t voiceLeadRoll = random(0, 100);
            if (voiceLeadRoll < 60) targetBassNote = activeMelodyBaseNote - 24;
            else if (voiceLeadRoll < 85) targetBassNote = activeMelodyBaseNote - 17;
            else targetBassNote = activeMelodyBaseNote - 12;
        } else {
            uint8_t rawNote = scaleMinor[currentChordIndex][0];
            if (currentArtistStyle == 1) rawNote = scaleDorian[currentChordIndex][0];
            else if (currentArtistStyle == 2) rawNote = scaleMajor[currentChordIndex][0];
            else if (currentArtistStyle == 3) rawNote = scaleAbstract[currentChordIndex][0];
            else if (currentArtistStyle == 4) rawNote = scaleAeolian[currentChordIndex][0];
            if (random(0, 100) < 25) rawNote = scaleMinor[currentChordIndex][1];
            targetBassNote = CLAMP(rawNote + globalKeyTransposition - 36, 12, 90);
        }
        if (longBarCounter % 2 == 0 && stepBase > 0 && random(0, 100) < 35) {
            targetBassNote += 12;
        }
        uint8_t bassVelocity = (bassMelodyInherit) ? 58 : 42;
        if (stepBase == 0) bassVelocity += random(5, 12);
        lastDroneNotes[matrixIdx] = targetBassNote;
        midiMsg(0x90 | (activeBassChannel - 1), lastDroneNotes[matrixIdx], bassVelocity);
        uint8_t bassFilterAccent = 40 + (stepBase * 10) + random(0, 15);
        midiMsg(0xB0 | (activeBassChannel - 1), 83, CLAMP(bassFilterAccent, 20, 110));
    }
}

ALWAYS INLINE void processMelodicEngine(uint8_t stepBase, uint8_t stepQuad, uint8_t beatsPerBar, uint8_t doubleBar, uint8_t quadBar) {
    uint8_t synthIdx = activeSynthChannel - SYNTH_START;
    if (synthIdx >= 4) return;

    if (!systemMuteArray[activeSynthChannel]) {
        uint8_t actualGoaPercent = 0;
        if (goaMotiveChance == 1)      actualGoaPercent = 10;
        else if (goaMotiveChance == 2) actualGoaPercent = 25;
        else if (goaMotiveChance == 3) actualGoaPercent = 50;
        else if (goaMotiveChance == 4) actualGoaPercent = 85;

        bool triggerGoaMotive = (random(0, 100) < actualGoaPercent);
        uint8_t dynamicTriggerChance = dynamicArpChance;

        if (currentMelodyStyle == 2)      dynamicTriggerChance = dynamicArpChance + 15;
        else if (currentMelodyStyle == 3) dynamicTriggerChance = 25;

        if (stepQuad >= doubleBar && currentMelodyStyle != 2) {
	    dynamicTriggerChance = (dynamicTriggerChance * 85) >> 8;
        }

        if (currentArtistStyle == 4) {
            dynamicTriggerChance = (stepBase != 0) ? 0 : 95;
        }

        if (random(0, 100) < dynamicTriggerChance) {
            if (lastArpNotes[synthIdx] > 0) {
                midiMsg(0x80 | (activeSynthChannel - 1), lastArpNotes[synthIdx], 0);
                lastArpNotes[synthIdx] = 0;
                goaNoteDurationCounter = 0;
            }

            if (triggerGoaMotive) {
                lastScalePositionIndex = (stepBase % 2 == 0) ? 0 : random(0, 7);
            } else {
                bool phraseTailResolution = (stepQuad >= (quadBar - beatsPerBar));
                if (phraseTailResolution && random(0, 100) < 75) {
                    uint8_t stableNotes[3] = {0, 2, 4};
                    lastScalePositionIndex = stableNotes[random(0, 3)];
                } else {
                    if (currentMelodyStyle == 0) {
                        if (random(0, 100) < 20) melodyDirection *= -1;
                        int8_t proposedIndex = lastScalePositionIndex + melodyDirection;
                        if (proposedIndex < 0) { proposedIndex = 1; melodyDirection = 1; }
                        if (proposedIndex > 6) { proposedIndex = 5; melodyDirection = -1; }
                        lastScalePositionIndex = (uint8_t)proposedIndex;
                    }
                    else if (currentMelodyStyle == 1) {
                        lastScalePositionIndex = (lastScalePositionIndex <= 2) ? random(4, 7) : random(0, 3);
                    }
                    else if (currentMelodyStyle == 2) {
                        lastScalePositionIndex = (tickCount % 2 == 0) ? 2 : 4;
                    }
                }
            }

            uint8_t baseNote = 0;
            if (triggerGoaMotive) {
                baseNote = scaleGoa[currentChordIndex][lastScalePositionIndex];
            } else {
                if (currentArtistStyle == 1)      baseNote = scaleDorian[currentChordIndex][lastScalePositionIndex];
                else if (currentArtistStyle == 2) baseNote = scaleMajor[currentChordIndex][lastScalePositionIndex];
                else if (currentArtistStyle == 3) baseNote = scaleAbstract[currentChordIndex][lastScalePositionIndex];
                else if (currentArtistStyle == 4) baseNote = scaleAeolian[currentChordIndex][lastScalePositionIndex];
                else                              baseNote = scaleMinor[currentChordIndex][lastScalePositionIndex];
            }

            baseNote = CLAMP(baseNote + globalKeyTransposition, 24, 110);
            activeMelodyBaseNote = baseNote;

            uint8_t octaveShift = 12;
            if (currentMelodyStyle == 1) {
                octaveShift = (lastScalePositionIndex <= 2) ? 12 : 36;
            } else if (currentMelodyStyle == 3) {
                octaveShift = 12;
            } else {
                if (stepQuad < beatsPerBar)            octaveShift = 24;
                else if (stepQuad < (beatsPerBar * 2)) octaveShift = 36;
                else                                   octaveShift = 12;
            }

            uint8_t targetMidiNote = CLAMP(baseNote + octaveShift, 24, 127);
            uint8_t expressiveVelocity = 55 + (stepBase * 3) + random(0, 12);
            if (triggerGoaMotive)        expressiveVelocity += 25;
            if (currentMelodyStyle == 3) expressiveVelocity = 45;

            if (currentMelodyStyle == 3) {
                midiMsg(0xB0 | (activeSynthChannel - 1), 72, random(80, 115));
            } else if (stepBase == 0) {
                midiMsg(0xB0 | (activeSynthChannel - 1), 72, 55);
            }

            uint8_t synthPan = 64 + random(-activePanIntensity, activePanIntensity + 1);
            midiMsg(0xB0 | (activeSynthChannel - 1), 10, CLAMP(synthPan, 10, 118));

            midiMsg(0x90 | (activeSynthChannel - 1), targetMidiNote, expressiveVelocity);
            lastArpNotes[synthIdx] = targetMidiNote;
            goaNoteDurationCounter = 0;
        } else {
            if (lastArpNotes[synthIdx] > 0) {
                goaNoteDurationCounter++;
                bool shouldTurnOff = false;
                switch (goaMotiveDuration) {
                    case 0: shouldTurnOff = true; break;
                    case 1: if (goaNoteDurationCounter >= 1) shouldTurnOff = true; break;
                    case 2: if (goaNoteDurationCounter >= 2) shouldTurnOff = true; break;
                    case 3: if (goaNoteDurationCounter >= 4) shouldTurnOff = true; break;
                    case 4: shouldTurnOff = false; break;
                }
                if (shouldTurnOff) {
                    midiMsg(0x80 | (activeSynthChannel - 1), lastArpNotes[synthIdx], 0);
                    lastArpNotes[synthIdx] = 0;
                    goaNoteDurationCounter = 0;
                }
            }
        }
    }
}


ALWAYS INLINE uint8_t generateFractalNoise() {
    static uint16_t x = 0xBEEFu;
    x ^= x >> 7; x ^= x << 9; x ^= x >> 13;
    uint8_t rawMod = x;
    while (rawMod >= 40) rawMod -= 40;
    return rawMod + 45;
}

ALWAYS INLINE void runPsybientEngine() {
    uint8_t beatsPerBar = currentTimeSignature;
    uint8_t doubleBar   = beatsPerBar * 2;
    uint8_t quadBar     = beatsPerBar * 4;

    uint8_t stepBase   = tickCount % beatsPerBar;
    uint8_t stepQuad   = tickCount % quadBar;

    if (stepQuad == (quadBar - 1)) {
        processMicroEvolution();

        if (longBarCounter % 16 == 0) {
            bassMelodyInherit = (random(0, 100) < 30);
            currentMelodyStyle = random(0, 4);
        }

        if (longBarCounter % 32 == 0) {
            uint8_t keyRoll = random(0, 100);
            if (keyRoll < 50)       globalKeyTransposition = 0;
            else if (keyRoll < 75)  globalKeyTransposition = 5;
            else                    globalKeyTransposition = 7;
        }
    }

    // --- MARKOV GENERATOR STEP ---
    if (stepQuad == 0) {
        uint8_t roll = random(0, 100); uint8_t sum = 0;
        for (uint8_t target = 0; target < currentTimeSignature; target++) {
            sum += markovMatrix[currentChordIndex][target];
            if (roll <= sum) { currentChordIndex = target; break; }
        }
    }

    // --- AUTOMATION TRACK ---
    if (tickCount % (beatsPerBar == 4 ? 4 : 3) == 0) { 
        uint8_t ccValue = generateFractalNoise();
        uint8_t longWaveDrift = (longBarCounter % 128) / 4;
        midiMsg(0xB0 | (activeSynthChannel - 1), 74, ccValue + 5 + longWaveDrift);
        midiMsg(0xB0 | (activeBassChannel - 1),  83, ccValue - 20 + (longWaveDrift / 2));
    }

    // --- DISPATCH SEPARATED INSTRUMENT SUB-ROUTINES ---
    processMelodicEngine(stepBase, stepQuad, beatsPerBar, doubleBar, quadBar);
    processBassEngine(stepBase);
    processPercussionEngine(stepBase, beatsPerBar);

    tickCount++;
    if (tickCount >= quadBar) {
        tickCount = 0; 
    }
}

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;

    for (int i = 0; i < 4; i++) {
        lastArpNotes[i] = 0;
        lastDroneNotes[i] = 0;
    }

    api->serialOpen(31250, -1, -1);;
    api->clear();
    api->color(MAGENTA);
    api->printAt(5, 1, STRING("bpm"));
    api->printAt(5, 2, STRING("pattern"));
    api->printAt(5, 3, STRING("style"));
    api->printAt(5, 4, STRING("synth"));
    api->printAt(5, 5, STRING("bass"));
    api->printAt(5, 6, STRING("drum"));
    api->printAt(5, 7, STRING("hats"));
    api->printAt(5, 8, STRING("bar"));
    transitionToNextArtist();
    while (api->inkey() != '\x13') {
        runPsybientEngine();
        api->delay(stepIntervalMs);
    }
    silenceAllChannels();
    api->clear();
    return 0;
}

