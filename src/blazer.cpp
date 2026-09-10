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

#include <exception>
#include <stdexcept>
#include <iostream>
#include <memory>

#include "config.h"
#include "mimetypes.h"
#include "exceptions.h"
#include "khi/argy/cli.h"

#include "command_create_bucket.h"
#include "command_delete_bucket.h"
#include "command_update_bucket.h"
#include "command_list_buckets.h"
#include "command_upload_file.h"
#include "command_ls.h"
#include "command_file_by_id.h"
#include "command_file_by_name.h"
#include "command_get_file_info.h"
#include "command_hide_file.h"
#include "command_list_file_versions.h"
#include "command_delete_file_version.h"

using namespace khi::argy;
using namespace khi::command;

int main(int argc, const char* argv[]) {

   Parser parser("blazer", "Backblaze B2 from the command line.", PACKAGE_VERSION);

   parser.add_command(std::make_unique<CreateBucket>());
   parser.add_command(std::make_unique<DeleteBucket>());
   parser.add_command(std::make_unique<UpdateBucket>());
   parser.add_command(std::make_unique<ListBuckets>());
   parser.add_command(std::make_unique<UploadFile>());
   parser.add_command(std::make_unique<Ls>());
   parser.add_command(std::make_unique<FileById>());
   parser.add_command(std::make_unique<FileByName>());
   parser.add_command(std::make_unique<GetFileInfo>());
   parser.add_command(std::make_unique<HideFile>());
   parser.add_command(std::make_unique<ListFileVersions>());
   parser.add_command(std::make_unique<DeleteFileVersion>());

   try {
      return parser.parse_and_run(argc, argv);
   } catch (const khi::ResponseError& err) {
      std::cerr << err.what() << std::endl;
      return EXIT_FAILURE;
   } catch (const std::runtime_error& err) {
      std::cerr << "ERROR: " << err.what() << std::endl;
      return EXIT_FAILURE;
   }
}
