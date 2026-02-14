/**
 * libpsd - Photoshop file formats (*.psd) decode library
 * Copyright (C) 2004-2007 Graphest Software.
 *
 * psd_ockham - Photoshop file size reducing utility
 * Copyright (C) 2017-2018 Playrix.
 * Modified by RezoliT (@Manifest_of_Destiny)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_MSC_VER)
#include <windows.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#endif

#if defined(__APPLE__)
#include <unistd.h>
#include <sys/uio.h>
#define fstat64 fstat
#define stat64 stat
#define lseeki64 lseek
#elif defined(_MSC_VER)
#include <io.h>
#define open _open
#define read _read
#define close _close
#define write _write
#define stat64 _stat64
#define lseeki64 _lseeki64
#define fstat64 _fstat64
#else // Unux
#define _LARGEFILE64_SOURCE
#include <sys/io.h>
#include <unistd.h>
#define stat64 stat
#define lseeki64 lseek
#define fstat64 fstat
#endif

#include <sys/types.h>
#include <fcntl.h>
#include "psd.h"
#include "psd_system.h"

#include <string.h>


#if defined(_MSC_VER)

static wchar_t* psd_path_to_wide(const char* s)
{
	if (s == NULL) return NULL;

	// 1) пробуем как UTF-8
	int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
	UINT cp = CP_UTF8;

	// 2) если не UTF-8 — fallback на системную ANSI codepage
	if (wlen == 0) {
		cp = CP_ACP;
		wlen = MultiByteToWideChar(cp, 0, s, -1, NULL, 0);
		if (wlen == 0) return NULL;
	}

	wchar_t* ws = (wchar_t*)malloc(sizeof(wchar_t) * (size_t)wlen);
	if (!ws) return NULL;

	if (MultiByteToWideChar(cp, 0, s, -1, ws, wlen) == 0) {
		free(ws);
		return NULL;
	}

	return ws;
}

#endif

void * psd_malloc(psd_int size)
{
	return malloc(size);
}

void * psd_realloc(void * block, psd_int size)
{
	return realloc(block, size);
}

void psd_free(void * block)
{
	free(block);
}

void psd_freeif(void * block)
{
	if (block != NULL)
		psd_free(block);
}

psd_int psd_fopen(const psd_char* file_name)
{
	psd_int flags = O_RDONLY;
#if defined(_MSC_VER)
	flags |= O_BINARY;

	wchar_t* wpath = psd_path_to_wide(file_name);
	if (!wpath) return -1;

	psd_int f = _wopen(wpath, flags);
	free(wpath);
	return f;
#else
	psd_int f = open(file_name, flags);
	return f;
#endif
}


psd_int psd_fopenw(const psd_char* file_name)
{
	psd_int flags = O_RDWR | O_CREAT;
#if defined(_MSC_VER)
	flags |= O_BINARY;

	wchar_t* wpath = psd_path_to_wide(file_name);
	if (!wpath) return -1;

	psd_int f = _wopen(wpath, flags, 0666);
	free(wpath);
	return f;
#else
	psd_int f = open(file_name, flags, 0666);
	return f;
#endif
}


psd_long psd_fsize(psd_int file)
{
	struct stat64 st = {0};
	if (fstat64(file, &st) == 0)
		return st.st_size;

	return -1;
}

psd_int psd_fread(psd_uchar * buffer, psd_int count, psd_int file)
{
	psd_int result = read(file, buffer, count);
	return result;
}

psd_long _psd_fseek(psd_int file, psd_long length, psd_int origin)
{
	psd_long result = lseeki64(file, length, origin);
	return result;
}

psd_long psd_fseek_set(psd_int file, psd_long length)
{
	return _psd_fseek(file, length, 0);
}

psd_long psd_fseek_end(psd_int file, psd_long length)
{
	return _psd_fseek(file, length, 2);
}

psd_long psd_ftell(psd_int file)
{
	psd_long result = _psd_fseek(file, 0, 1);
	return result;
}

psd_int psd_fwrite(psd_uchar * buffer, psd_int count, psd_int file)
{
	psd_int result = write(file, buffer, count);
	return result;
}

void psd_fclose(psd_int file)
{
	close(file);
}

psd_int psd_remove(const psd_char* file_name)
{
#if defined(_MSC_VER)
	wchar_t* wpath = psd_path_to_wide(file_name);
	if (!wpath) return -1;
	int r = _wremove(wpath);
	free(wpath);
	return r;
#else
	return remove(file_name);
#endif
}

psd_int psd_rename(const psd_char* old_name, const psd_char* new_name)
{
#if defined(_MSC_VER)
	wchar_t* wold = psd_path_to_wide(old_name);
	wchar_t* wnew = psd_path_to_wide(new_name);
	if (!wold || !wnew) {
		if (wold) free(wold);
		if (wnew) free(wnew);
		return -1;
	}
	int r = _wrename(wold, wnew);
	free(wold);
	free(wnew);
	return r;
#else
	return rename(old_name, new_name);
#endif
}
