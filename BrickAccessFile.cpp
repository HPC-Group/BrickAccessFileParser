#include "BrickAccessFile.h"

#include <iostream>
#include <sstream>

namespace {

  enum ProtectMode {
    PM_NONE = 0,
    PM_QUOTES,
    PM_BRACKETS,
    PM_CUSTOM_DELIMITER
  };

  std::vector<std::string> Tokenize(
    const std::string& strInput,
    ProtectMode mode = PM_QUOTES,
    char customOrOpeningDelimiter='{',
    char closingDelimiter='}')
  {
    std::vector<std::string> strElements;
    switch (mode) {
    case PM_QUOTES :
      {
        std::string buf;
        std::stringstream ss(strInput);
        bool bProtected = false;
        while (ss >> buf) {
          std::string cleanBuf = buf;
          if (cleanBuf[0] == '\"')
            cleanBuf = cleanBuf.substr(1, cleanBuf.length()-1);

          if (!cleanBuf.empty() && cleanBuf[cleanBuf.size()-1] == '\"')
            cleanBuf = cleanBuf.substr(0, cleanBuf.length()-1);

          if (bProtected) {
            size_t end = strElements.size()-1;
            strElements[end] = strElements[end] + " " + cleanBuf;
          }
          else
            strElements.push_back(cleanBuf);

          if (buf[0] == '\"')
            bProtected = true;
          if (buf[buf.size()-1] == '\"')
            bProtected = false;
        }
      } break;

    case PM_BRACKETS :
      {
        int iLevel = 0;
        size_t iStart = 0;
        size_t i = 0;
        for (;i<strInput.size();i++) {
          if (strInput[i] == customOrOpeningDelimiter) {
            if (iLevel == 0)
              iStart++;
            iLevel++;
            continue;
          } else if (strInput[i] == closingDelimiter) {
            iLevel--;
            if (iLevel == 0) {
              if (i-iStart > 0)
                strElements.push_back(strInput.substr(iStart, i-iStart));
              iStart = i+1;
            }
            continue;
          }
          switch (strInput[i]) {
          case L' ':
          case L'\n':
          case L'\r':
          case L'\t':
            if (iLevel == 0) {
              if (i-iStart > 0)
                strElements.push_back(strInput.substr(iStart, i-iStart));
              iStart = i+1;
            }
            break;
          }
        }
        if (i-iStart > 0)
          strElements.push_back(strInput.substr(iStart, i-iStart));
      } break;

    case PM_CUSTOM_DELIMITER :
      {
        size_t iStart = 0;
        size_t i = 0;
        for (;i<strInput.size();i++) {
          if (strInput[i] == customOrOpeningDelimiter) {
            if (i-iStart > 0)
              strElements.push_back(strInput.substr(iStart, i-iStart));
            iStart = i+1;
          }
        }
        if (i-iStart > 0)
          strElements.push_back(strInput.substr(iStart, i-iStart));
      } break;

    default :
      {
        std::string buf;
        std::stringstream ss(strInput);
        while (ss >> buf) strElements.push_back(buf);
      } break;
    }
    return strElements;
  }

  template <typename T> T FromString(
    const std::string& s,
    std::ios_base& (*f)(std::ios_base&) = std::dec)
  {
    T t;
    std::istringstream iss(s);
    iss >> f >> t;
    return t;
  }

}

BrickAccessFile::BrickAccessFile(std::string const& filename)
  : m_File(filename)
{}

bool BrickAccessFile::Load()
{
  if (!m_File.is_open()) return false;

  m_Frames.clear();
  uint32_t iExpectedBricks = 0;
  uint32_t iSubframeCounter = 0;
  uint32_t iFrameCounter = 0;

  std::vector<std::string> tokens, elements;
  std::string line;
  for (uint32_t i=1; getline(m_File, line); ++i) {
    if (line.size() > 0) {
      if (line[0] == '#' )
        continue; // skip comment line

      if (line[0] == '[') {
        Frame& frame = m_Frames.back();
        Subframe& subframe = frame.back();

        tokens = Tokenize(line, PM_BRACKETS, '[', ']');
        if (tokens.size() != iExpectedBricks) {
          std::cerr << "failed to parse line " << i << ": wrong brick count for subframe" << std::endl;
          return false;
        }
        for (size_t t=0; t<tokens.size(); ++t) {
          elements = Tokenize(tokens[t], PM_NONE);
          if (elements.size() != 4) {
            std::cerr << "failed to parse line " << i << " at brick number: " << t << std::endl;
            return false;
          }
          subframe.push_back(Brick());
          Brick& brick = subframe.back();
          brick.x = FromString<uint32_t>(elements[0]);
          brick.y = FromString<uint32_t>(elements[1]);
          brick.z = FromString<uint32_t>(elements[2]);
          brick.w = FromString<uint32_t>(elements[3]);
          if (brick.w >= m_BrickCounts.size()) {
            std::cerr << "failed to parse line " << i << " at brick number: " << t << " brick LoD exceeds bounds" << std::endl;
            return false;
          }
          if (brick.x >= m_BrickCounts[size_t(brick.w)].x ||
            brick.y >= m_BrickCounts[size_t(brick.w)].y ||
            brick.z >= m_BrickCounts[size_t(brick.w)].z) {
              std::cerr << "failed to parse line " << i << " at brick number: " << t << " brick position exceeds bounds" << std::endl;
              return false;
          }
        }
        iSubframeCounter++;
        continue; // finished parsing subframe
      }

      // parse header, subframe and frame marks
      tokens = Tokenize(line, PM_CUSTOM_DELIMITER, '=');
      if (tokens.size() > 1) {
        if (tokens[0] == "Filename") {
          // unused parameter
          //m_Filename = tokens[1];

        } else if (tokens[0] == "MaxBrickSize") {
          tokens = Tokenize(tokens[1], PM_NONE);
          if (tokens.size() != 3) {
            std::cerr << "failed to parse line " << i << ": invalid MaxBrickSize" << std::endl;
            return false;
          }
          m_MaxBrickSize.x = FromString<uint32_t>(tokens[0]);
          m_MaxBrickSize.y = FromString<uint32_t>(tokens[1]);
          m_MaxBrickSize.z = FromString<uint32_t>(tokens[2]);

        } else if (tokens[0] == "BrickOverlap") {
          tokens = Tokenize(tokens[1], PM_NONE);
          if (tokens.size() != 3) {
            std::cerr << "failed to parse line " << i << ": invalid BrickOverlap" << std::endl;
            return false;
          }
          m_BrickOverlap.x = FromString<uint32_t>(tokens[0]);
          m_BrickOverlap.y = FromString<uint32_t>(tokens[1]);
          m_BrickOverlap.z = FromString<uint32_t>(tokens[2]);

        } else if (tokens[0] == "LoDCount") {
          if (tokens.size() != 2) {
            std::cerr << "failed to parse line " << i << ": invalid LoDCount" << std::endl;
            return false;
          }
          uint32_t const LoDCount = FromString<uint32_t>(tokens[1]);
          m_DomainSizes.resize(LoDCount);
          m_BrickCounts.resize(LoDCount);

        } else if (tokens[0] == " LoD" || tokens[0] == "LoD") {
          if (tokens.size() != 4) {
            std::cerr << "failed to parse line " << i << ": invalid LoD" << std::endl;
            return false;
          }
          elements = Tokenize(tokens[1], PM_NONE);
          if (elements.size() != 2) {
            std::cerr << "failed to parse line " << i << ": invalid LoD value" << std::endl;
            return false;
          }
          uint32_t const LoD = FromString<uint32_t>(elements[0]);
          if (LoD >= m_DomainSizes.size() || LoD >= m_BrickCounts.size()) {
            std::cerr << "failed to parse line " << i << ": LoD not available" << std::endl;
            return false;
          }
          if (elements[1] != "DomainSize") {
            std::cerr << "failed to parse line " << i << ": DomainSize not available" << std::endl;
            return false;
          }
          elements = Tokenize(tokens[2], PM_NONE);
          if (elements.size() != 4) {
            std::cerr << "failed to parse line " << i << ": invalid DomainSize" << std::endl;
            return false;
          }
          m_DomainSizes[LoD].x = FromString<uint32_t>(elements[0]);
          m_DomainSizes[LoD].y = FromString<uint32_t>(elements[1]);
          m_DomainSizes[LoD].z = FromString<uint32_t>(elements[2]);
          if (elements[3] != "BrickCount") {
            std::cerr << "failed to parse line " << i << ": BrickCount not available" << std::endl;
            return false;
          }
          elements = Tokenize(tokens[3], PM_NONE);
          if (elements.size() != 3) {
            std::cerr << "failed to parse line " << i << ": invalid BrickCount" << std::endl;
            return false;
          }
          m_BrickCounts[LoD].x = FromString<uint32_t>(elements[0]);
          m_BrickCounts[LoD].y = FromString<uint32_t>(elements[1]);
          m_BrickCounts[LoD].z = FromString<uint32_t>(elements[2]);

        } else if (tokens[0] == " Subframe" || tokens[0] == "Subframe") {
          if (tokens.size() != 3) {
            std::cerr << "failed to parse line " << i << ": invalid Subframe" << std::endl;
            return false;
          }
          elements = Tokenize(tokens[1], PM_NONE);
          if (elements.size() != 2) {
            std::cerr << "failed to parse line " << i << ": invalid Subframe value" << std::endl;
            return false;
          }
          uint32_t const iSubframe = FromString<uint32_t>(elements[0]);
          if (iSubframe != iSubframeCounter) {
            std::cerr << "failed to parse line " << i << ": wrong Subframe value" << std::endl;
            return false;
          }
          iExpectedBricks = FromString<uint32_t>(tokens[2]);
          while (m_Frames.size() <= iFrameCounter)
            m_Frames.push_back(Frame());
          m_Frames.back().push_back(Subframe());

        } else if (tokens[0] == " Frame" || tokens[0] == "Frame") {
          if (tokens.size() != 4) {
            std::cerr << "failed to parse line " << i << ": invalid Frame" << std::endl;
            return false;
          }
          std::vector<std::string> elements;
          elements = Tokenize(tokens[1], PM_NONE);
          if (elements.size() != 2) {
            std::cerr << "failed to parse line " << i << ": invalid Frame value" << std::endl;
            return false;
          }
          uint32_t const iFrame = FromString<uint32_t>(elements[0]);
          if (iFrame != iFrameCounter) {
            std::cerr << "failed to parse line " << i << ": wrong Frame value" << std::endl;
            return false;
          }
          while (m_Frames.size() <= iFrameCounter)
            m_Frames.push_back(Frame());
          iFrameCounter++;
          iExpectedBricks = 0;
          iSubframeCounter = 0;
        }
      }
    }
  }
  return true;
}

BrickAccessFile::Vec3<uint32_t> const& BrickAccessFile::GetMaxBrickSize() const
{
  return m_MaxBrickSize;
}

BrickAccessFile::Vec3<uint32_t> const& BrickAccessFile::GetBrickOverlap() const
{
  return m_BrickOverlap;
}

size_t BrickAccessFile::GetLoDCount() const
{
  return m_DomainSizes.size(); 
}

std::vector<BrickAccessFile::Vec3<uint64_t> > const& BrickAccessFile::GetDomainSizes() const
{
  return m_DomainSizes;
}

std::vector<BrickAccessFile::Vec3<uint64_t> > const& BrickAccessFile::GetBrickCounts() const
{
  return m_BrickCounts;
}

std::vector<BrickAccessFile::Frame> const& BrickAccessFile::GetFrames() const
{
  return m_Frames;
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
