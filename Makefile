#
#   Copyright (c) 2025 Aftersol
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"), to deal
#   in the Software without restriction, including without limitation the rights
#   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#   copies of the Software, and to permit persons to whom the Software is
#   furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included in all
#   copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#   SOFTWARE.
#

V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

all: qoi.z64
.PHONY: all
FILESYSTEM_DIR = filesystem
assets = $(wildcard $(FILESYSTEM_DIR)/*.qoi)

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/qoi_viewer.o

qoi.z64: N64_ROM_TITLE="qoiImageViewer"
qoi.z64: $(BUILD_DIR)/qoi.dfs

$(BUILD_DIR)/qoi.elf: $(OBJS)
$(BUILD_DIR)/qoi.dfs: $(assets)
	@echo "	[DFS] $@"
	if [ ! -s "$<"]; then rm -f "$<"; fi
	$(N64_MKDFS) "$@" filesystem >/dev/null

clean:
	rm -f $(BUILD_DIR)/* *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)