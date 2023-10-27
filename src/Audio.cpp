/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#include "Audio.h"
#include "Memory.h"

Audio::Audio()
{
    m_ElapsedCycles = 0;
#if !defined(SF2000)
    m_iSampleRate = 44100;
#else
    m_iSampleRate = 11025;
#endif
    InitPointer(m_pApu);
    InitPointer(m_pBuffer);
    InitPointer(m_pSampleBuffer);
    m_bPAL = false;
}

Audio::~Audio()
{
    SafeDelete(m_pApu);
    SafeDelete(m_pBuffer);
    SafeDeleteArray(m_pSampleBuffer);
}

void Audio::Init()
{
    m_pSampleBuffer = new blip_sample_t[GS_AUDIO_BUFFER_SIZE];

    m_pApu = new Sms_Apu();
    m_pBuffer = new Stereo_Buffer();

    // Clock rate for NTSC is 3579545
    // Clock rate for PAL is 3546893
    m_pBuffer->clock_rate(m_bPAL ? 3546893 : 3579545);
    m_pBuffer->set_sample_rate(m_iSampleRate);

    //m_pApu->treble_eq(-15.0);
    //m_pBuffer->bass_freq(100);

    m_pApu->output(m_pBuffer->center(), m_pBuffer->left(), m_pBuffer->right());
}

void Audio::Reset(bool bPAL)
{
    m_bPAL = bPAL;
    m_pApu->reset();
    m_pBuffer->clear();
    m_pBuffer->clock_rate(m_bPAL ? 3546893 : 3579545);
    m_ElapsedCycles = 0;
}

void Audio::SetSampleRate(int rate)
{
    if (rate != m_iSampleRate)
    {
        m_iSampleRate = rate;
        m_pBuffer->set_sample_rate(m_iSampleRate);
    }
}

void Audio::SetVolume(float volume)
{
    m_pApu->volume(volume);
}

void Audio::EndFrame(s16* pSampleBuffer, int* pSampleCount)
{
    m_pApu->end_frame(m_ElapsedCycles);
    m_pBuffer->end_frame(m_ElapsedCycles);

    int count = static_cast<int>(m_pBuffer->read_samples(m_pSampleBuffer, GS_AUDIO_BUFFER_SIZE));

    if (IsValidPointer(pSampleBuffer) && IsValidPointer(pSampleCount))
    {
        *pSampleCount = count;

        for (int i=0; i<count; i++)
        {
            pSampleBuffer[i] = m_pSampleBuffer[i];
        }
    }

    m_ElapsedCycles = 0;
}

void Audio::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_ElapsedCycles), sizeof(m_ElapsedCycles));
    stream.write(reinterpret_cast<const char*> (m_pSampleBuffer), sizeof(blip_sample_t) * GS_AUDIO_BUFFER_SIZE);

}

void Audio::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_ElapsedCycles), sizeof(m_ElapsedCycles));
    stream.read(reinterpret_cast<char*> (m_pSampleBuffer), sizeof(blip_sample_t) * GS_AUDIO_BUFFER_SIZE);

    m_pApu->reset();
    m_pBuffer->clear();
}
