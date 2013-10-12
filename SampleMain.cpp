#include <iostream>
#include <string>

#include "BrickAccessFile.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

string GetFilename(const std::string& filename)
{
  size_t index = std::max(size_t(filename.find_last_of("\\")), size_t(filename.find_last_of("/")))+1;
  string name = filename.substr(index,filename.length()-index);
  return name;
}

// Simple example program to demonstrate the BrickAccessFile class and print
// some values of the loaded file.
//
// This code is part of the supplementary material of the paper:
//
// Thomas Fogal, Alexander Schiewe and Jens Krueger.
// An Analysis of Scalable GPU-Based Ray-Guided Volume Rendering.
// In Proceedings of IEEE Large Scale Data Analysis and Visualization Symposium
// (LDAV), 2013.
//
// Please see the paper for further details.
int main(int argc, char const *argv[])
{
  if (argc != 2) {
    string const arg0(argv[0]);
    cerr << "usage: " << GetFilename(arg0) << " filename" << endl;
    return EXIT_FAILURE;
  }

  string const arg1(argv[1]);

  BrickAccessFile baf(arg1);

  if (!baf.Load()) {
    return EXIT_FAILURE;
  }

  BrickAccessFile::Vec3<uint32_t> const& maxBrickSize = baf.GetMaxBrickSize();
  BrickAccessFile::Vec3<uint32_t> const& brickOverlap = baf.GetBrickOverlap();
  size_t const lodCount = baf.GetLoDCount();
  cout << "max brick size:   " << maxBrickSize.x << "x" << maxBrickSize.y << "x" << maxBrickSize.z << endl;
  cout << "brick overlap:    " << brickOverlap.x << "x" << brickOverlap.y << "x" << brickOverlap.z << endl;
  cout << "levels of detail: " << lodCount << endl;

  for (size_t lod=0; lod<lodCount; ++lod) {
    BrickAccessFile::Vec3<uint64_t> const& domainSize = baf.GetDomainSizes()[lod];
    BrickAccessFile::Vec3<uint64_t> const& brickCount = baf.GetBrickCounts()[lod];
    cout << "level of detail: " << lod;
    cout << " domain size: " << domainSize.x << "x" << domainSize.y << "x" << domainSize.z;
    cout << " brick count: " << brickCount.x << "x" << brickCount.y << "x" << brickCount.z << endl;
  }
  
  size_t totalFrameCount = 0;
  size_t totalSubframeCount = 0;
  size_t totalBrickCount = 0;

  // This is the interesting part where we access the brick access patterns.
  // You might want to use this data, for example to benchmark your own data
  // structures with real world data indices provided at our supplementary
  // material webpage.
  for (auto frame=baf.GetFrames().cbegin(); frame!=baf.GetFrames().cend(); ++frame) {
    for (auto subframe=frame->cbegin(); subframe!=frame->cend(); ++subframe) {
      for (auto brick=subframe->cbegin(); brick!=subframe->cend(); ++brick) {
        ++totalBrickCount;
      }
      ++totalSubframeCount;
    }
    ++totalFrameCount;
  }

  cout << "total frame count:    " << totalFrameCount << endl;
  cout << "total subframe count: " << totalSubframeCount << endl;
  cout << "total brick count:    " << totalBrickCount << endl;

  return EXIT_SUCCESS;
}

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
