#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc.h>
#include <objc/objc-runtime.h>
#include <iostream>
#import <Cocoa/Cocoa.h>
#include <string>
#include <vector>
// #include "Editor.h"

std::vector<std::string> directory_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save, bool multiple) {
    if (save && multiple) {
        throw std::invalid_argument("save and multiple must not both be true.");
    }

    std::vector<std::string> result;
    if (save) {
        NSSavePanel *saveDlg = [NSSavePanel savePanel];

        NSMutableArray *types = [NSMutableArray new];
        for (size_t idx = 0; idx < filetypes.size(); ++idx)
            [types addObject: [NSString stringWithUTF8String: filetypes[idx].first.c_str()]];

        [saveDlg setAllowedFileTypes: types];

        if ([saveDlg runModal] == NSModalResponseOK)
            result.emplace_back([[[saveDlg URL] path] UTF8String]);
    } else {
        NSOpenPanel *openDlg = [NSOpenPanel openPanel];

        [openDlg setCanChooseFiles:YES];
        [openDlg setCanChooseDirectories:YES];
        [openDlg setAllowsMultipleSelection:multiple];
        NSMutableArray *types = [NSMutableArray new];
        for (size_t idx = 0; idx < filetypes.size(); ++idx)
            [types addObject: [NSString stringWithUTF8String: filetypes[idx].first.c_str()]];

        [openDlg setAllowedFileTypes: types];

        if ([openDlg runModal] == NSModalResponseOK) {
            for (NSURL* url in [openDlg URLs]) {
                result.emplace_back((char*) [[url path] UTF8String]);
            }
        }
    }
    return result;
}