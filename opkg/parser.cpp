//
// Created by brant on 7/1/25.
//

#include "opkg.h"
#include <cstdio>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#include "zlib.h"
#include "utils.h"

inline bool try_parse_str(const char* prefix, const char *line, string &field){
    if (strncmp(line, prefix, strlen(prefix)) != 0)
        return false;
    auto f = string(line + strlen(prefix) + 1);
    utils::trim(f);
    field = f;
    return true;
}

inline bool try_parse_bool(const char *prefix, const char* line, bool &field){
    string s;
    if(try_parse_str(prefix, line, s)){
        if(strncmp(s.c_str(),"yes",3) == 0) {
            field = true;
            return true;
        }
    }
    return false;
}

inline bool try_parse_uint(const char *prefix, const char *line, uint &field) {
    string f;
    if (!try_parse_str(prefix, line, f))
        return false;
    try {
        field = stoul(f);
        return true;
    }
    catch (exception&) {
        return false;
    }
}

inline bool try_parse_long(const char *prefix, const char *line, long &field) {
    string f;
    if (!try_parse_str(prefix, line, f))
        return false;
    try {
        field = stol(f);
        return true;
    }
    catch (exception&) {
        return false;
    }
}

string lastLine;

//this is mostly copied from opkg's own parser. I don't hate it?
bool opkg::parse_line(shared_ptr<package> &ptr, const char *line, bool update, bool upstream) {
    static bool parsing_desc = false; //this is ugly, but it's what opkg does so whatever I guess
    static bool parsing_conf = false; //this is ugly, but it's what opkg does so whatever I guess
    if (ptr == nullptr)
        return false;
    switch (*line) {
        case 'P': {
            // PACKAGE NAME HANDLING
            if (!update) {
                if (try_parse_str("Package", line, ptr->Package)) {     //we're parsing packages out of multiple disparate lists
                    parsing_desc = false;                               //check if we already have a matching package
                    parsing_conf = false;                               //if we do, reset the package pointer to the extant package
                    break;                                              //then we can just keep processing as normal
                }                                                       //this works because Package is always the first line
            }                                                           //in the entry
            else {
                string pn;
                if (try_parse_str("Package", line, pn)) {
                    auto it = packages.find(pn);
                    if (it != packages.end()) {
                        auto &fpk = it->second;
                        ptr = fpk;
                        parsing_desc = false;
                        parsing_conf = false;
                        break;
                    } else {
                        //TODO: actually handle this error?
                        printf("ERROR! PACKAGE NOT FOUND! %s\n", pn.c_str());
                        break;
                    }
                }
            }
            // /PACKAGE NAME HANDLING
            if(try_parse_str("Provides", line, ptr->_provides_str)){
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'A': {
            if (try_parse_str("Architecture", line, ptr->Architecture)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Alternatives", line, ptr->Alternatives)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_bool("Auto-Installed", line, ptr->autoInstalled)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'D': {
            if (try_parse_str("Description", line, ptr->Description)) {
                parsing_desc = true;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Depends", line, ptr->_depends_str)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'C': {
            if(try_parse_str("Conflicts", line, ptr->_conflicts_str)){
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            string c;
            if (try_parse_str("CPE-ID", line, c)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Conffiles", line, c)) {  //we don't need conffiles, only one package has it
                parsing_desc = false;                   //this will create some errors on the following lines
                //but that's manageable
                parsing_conf = true;                    //the above was a lie, let's handle this anyway
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'E': {
            if (try_parse_bool("Essential", line, ptr->Essential)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'F': {
            if (try_parse_str("Filename", line, ptr->Filename)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'H': {
            if (try_parse_str("Homepage", line, ptr->Homepage)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'I': {
            if (try_parse_uint("Installed-Size", line, ptr->Size)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_long("Installed-Time", line, ptr->installTime)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'L': {
            if (try_parse_str("License", line, ptr->License)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'M': {
            if (try_parse_str("Maintainer", line, ptr->Maintainer)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'R': {
            if (try_parse_str("Replaces", line, ptr->_replaces_str)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            string r;
            if (try_parse_str("Require-User", line, r)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'S': {
            if (try_parse_str("Section", line, ptr->Section)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("SHA256sum", line, ptr->SHA256sum)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_uint("Size", line, ptr->Size)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Status", line, ptr->_status_str)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            string s;
            if (try_parse_str("SourceDateEpoch", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("SourceName", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            if (try_parse_str("Source", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'U':{
            //URL appears to be an alias for Homepage?
            if (try_parse_str("URL", line, ptr->Homepage)) {
                parsing_desc = false;
                parsing_conf = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'V': {
            string s;
            if (try_parse_str("Version", line, s)) {
                parsing_desc = false;
                parsing_conf = false;
                if(upstream)
                    ptr->UpstreamVersion = s;
                else
                    ptr->InstalledVersion = s;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case ' ': {
            if (parsing_desc) {
                //newlines in descriptions are prefixed with a space, apparently
                utils::trim(ptr->Description);
                ptr->Description.append("\n");
                auto ld = string(line);
                //printf("Appending line to description for package %s\n 1: %s\n 2: %s\n", ptr->Package.c_str(), ptr->Description.c_str(), ld.c_str());
                utils::trim(ld);
                ptr->Description.append(ld);
                break;
            }
            if (parsing_conf) {
                utils::trim(ptr->Conffiles);
                ptr->Conffiles.append("\n");
                auto ld = string(line);
                utils::trim(ld);
                ptr->Conffiles.append(ld);
                break;
            }
        }
        NOT_RECOGNIZED:
        default: {
            auto dln = strlen(line);
            if (dln <= 2)
                return false;
            for (uint i = 0; i < dln; i++)
                if (line[i] != ' ' && line[i] != '\n' && line[i] != '\r') {
                    printf("BadChar: %d :: %2x :: %c\n", i, line[0], line[0]);
                    printf("Unhandled tag: %s\n", line);
                    printf("Last good line: %s\n", lastLine.c_str());
                    return true;
                }
            printf("UNKN: ");
            for (uint j = 0; j < dln; j++) {
                printf("%2x ", line[j]);
            }
            printf("\n");
            return false;
        }
    }
    lastLine = line;
    return true;
}
