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

#ifndef COMMAND_FILE_BY_NAME_H
#define COMMAND_FILE_BY_NAME_H

#include "command.h"

namespace khi {
namespace command {

struct FileByName : BlazerCommand {

   Argument<std::string> bucket_name{this, "bucket-name", "Name of the bucket."};
   Argument<std::string> remote_file_name{this, "remote-file-name", "Name of the remote file."};
   Argument<std::string> local_file_path{this, "local-file-path", "Local path to save the file."};

   CommandConfiguration configuration() const override {
      return {"download_file_by_name", "Download a file by bucket and name.", "", ""};
   }

   int run() override;
};

} // namespace command
} // namespace khi

#endif // COMMAND_FILE_BY_NAME_H
