#pragma once

#ifndef BRICK_ACCESS_FILE_H
#define BRICK_ACCESS_FILE_H

#include <string>
#include <fstream>
#include <cstdint>
#include <vector>

// Helper class to load & parse ASCII-based pre-recorded brick access files
// (extension: *.ba) for further processing.
//
// This code is part of the supplementary material of the paper:
//
// Thomas Fogal, Alexander Schiewe and Jens Krueger.
// An Analysis of Scalable GPU-Based Ray-Guided Volume Rendering.
// In Proceedings of IEEE Large Scale Data Analysis and Visualization Symposium
// (LDAV), 2013.
//
// Please see the paper for further details.
class BrickAccessFile {
public:
  // C'tor to instantiate a BrickAccessFile from a given filename.
  BrickAccessFile(std::string const& filename);

  // Generic 3-component vector.
  template<class T>
  struct Vec3 {
    T x, y, z;
  };

  // Generic 4-component vector.
  template<class T>
  struct Vec4 {
    T x, y, z, w;
  };

  // A spatial brick position/index of a bricked domain. The w-component encodes
  // the level of detail.
  typedef Vec4<uint64_t> Brick;

  // A subframe consists of a bunch of brick indices.
  typedef std::vector<Brick> Subframe;

  // A frame consists of a bunch of subframes.
  typedef std::vector<Subframe> Frame;

  // Load & parse the brick access file.
  // @returns true if parsing finished without any problems. False indicates an
  //   error, see std::cerr for details.
  bool Load();

  // @returns the bricking size in voxels used for the spatial decomposition of
  //   the domain. The overlap between adjacent bricks is included in this brick
  //   size. The effective brick size with the actual payload can be computed as
  //   follows: coreBrickSize = maxBrickSize - 2 * brickOverlap.
  //   The actual used brick size can be much smaller, when, for example, the
  //   domain itself is smaller than the brick size requested for a particular
  //   level of detail.
  Vec3<uint32_t> const& GetMaxBrickSize() const;

  // @returns the overlap in voxels between adjacent bricks.
  Vec3<uint32_t> const& GetBrickOverlap() const;

  // @returns the number of levels of detail used by the dataset.
  size_t GetLoDCount() const;

  // @returns the domain size in voxels for every level of detail.
  std::vector<Vec3<uint64_t> > const& GetDomainSizes() const;

  // @returns the brick layout for every level of detail. This is the number of
  //   bricks which exist (given per-dimension).
  std::vector<Vec3<uint64_t> > const& GetBrickCounts() const;

  // @returns the captured brick access patterns indexed by frame and subframes.
  std::vector<Frame> const& GetFrames() const;

private:
  std::ifstream m_File;
  Vec3<uint32_t> m_MaxBrickSize;
  Vec3<uint32_t> m_BrickOverlap;
  std::vector<Vec3<uint64_t> > m_DomainSizes;
  std::vector<Vec3<uint64_t> > m_BrickCounts;
  std::vector<Frame> m_Frames;
};

#endif // BRICK_ACCESS_FILE_H

/*
   The MIT License (MIT)

   Copyright (c) 2013 HPC Group, Duisburg-Essen.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
