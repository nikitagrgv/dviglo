// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "sound_stream.h"

#include <memory>

namespace dviglo
{

class Sound;

/// Ogg Vorbis sound stream.
class DV_API OggVorbisSoundStream : public SoundStream
{
public:
    /// Construct from an Ogg Vorbis compressed sound.
    explicit OggVorbisSoundStream(const Sound* sound);
    /// Destruct.
    ~OggVorbisSoundStream() override;

    /// Seek to sample number. Return true on success.
    bool Seek(unsigned sample_number) override;

    /// Produce sound data into destination. Return number of bytes produced. Called by SoundSource from the mixing thread.
    unsigned GetData(signed char* dest, unsigned numBytes) override;

protected:
    /// Decoder state.
    void* decoder_;
    /// Compressed sound data.
    std::shared_ptr<signed char[]> data_;
    /// Compressed sound data size in bytes.
    unsigned dataSize_;
};

}
