#
# Copyright (C) 2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

import common
import re

def FullOTA_InstallEnd(info):
  input_zip = info.input_zip
  OTA_InstallEnd(info, input_zip)
  return

def IncrementalOTA_InstallEnd(info):
  input_zip = info.target_zip
  OTA_InstallEnd(info, input_zip)
  return

def AddImage(info, input_zip, basename, dest):
  path = "IMAGES/" + basename
  if path not in input_zip.namelist():
    return
  data = input_zip.read(path)
  common.ZipWriteStr(info.output_zip, basename, data)
  info.script.AppendExtra('package_extract_file("%s", "%s");' % (basename, dest))

def OTA_InstallEnd(info, input_zip):
  info.script.Print("Patching firmware images...")
  AddImage(info, input_zip, "vbmeta.img", "/dev/block/platform/bootdevice/by-name/vbmeta")
  AddImage(info, input_zip, "vbmeta_system.img", "/dev/block/platform/bootdevice/by-name/vbmeta_system")
  AddImage(info, input_zip, "vbmeta_vendor.img", "/dev/block/platform/bootdevice/by-name/vbmeta_vendor")
  AddImage(info, input_zip, "dtbo.img", "/dev/block/platform/bootdevice/by-name/dtbo")
  return
