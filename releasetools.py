#
# Copyright (C) 2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

import common
import re

def FullOTA_InstallEnd(info):
  OTA_InstallEnd(info)
  return

def IncrementalOTA_InstallEnd(info):
  OTA_InstallEnd(info)
  return

def AddImage(info, basename, dest):
  path = "IMAGES/" + basename
  if path not in info.input_zip.namelist():
    return
  data = info.input_zip.read(path)
  common.ZipWriteStr(info.output_zip, basename, data)
  info.script.AppendExtra('package_extract_file("%s", "%s");' % (basename, dest))

def OTA_InstallEnd(info):
  info.script.Print("Patching firmware images...")
  AddImage(info, "vbmeta.img", "/dev/block/platform/bootdevice/by-name/vbmeta")
  AddImage(info, "vbmeta_system.img", "/dev/block/platform/bootdevice/by-name/vbmeta_system")
  AddImage(info, "vbmeta_vendor.img", "/dev/block/platform/bootdevice/by-name/vbmeta_vendor")
  AddImage(info, "dtbo.img", "/dev/block/platform/bootdevice/by-name/dtbo")
  return
