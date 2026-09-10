// vim:set et ts=3 sw=3:
// __  __ ______ _______ _______ _______ ______
// |  |/  |   __ \   |   |     __|    ___|   __ \
// |     <|      <   |   |    |  |    ___|      <
// |__|\__|___|__|_______|_______|_______|___|__|
//        H E A V Y  I N D U S T R I E S
//
// Copyright (C) 2016 Kruger Heavy Industries
// http://www.krugerheavyindustries.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <list>
#include <memory>
#include <fstream>
#include <iostream>

#include "khi/argy/cli.h"

namespace khi {
class BB_Bucket;
class BB_Object;
class BB;
}

namespace khi {
namespace command {

using namespace khi::argy;

struct BlazerCommand : public Command {

   Option<std::string> credentials{this, "credentials", "c", "Path to credentials file.", std::string("")};
   Flag test_mode{this, "test-mode", "x", "Enable test mode."};

   std::unique_ptr<BB> createBB();

protected:

   void printBucket(const BB_Bucket& bucket, bool bucketName = false) const;

   void printObject(const BB_Object& object, bool longFormat = false) const;
};

} // namespace command
} // namespace khi

#endif // COMMAND_H
