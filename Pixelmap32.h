/*
  Pixelmap32.h

  Copyright (C) 2004-2013 by Gary Frattarola. All rights reserved.

  This code is distributed under the terms of the zlib license:

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Gary Frattarola
  gary.frattarola@gmail.com
*/

#ifndef _Pixelmap32_h_
#define _Pixelmap32_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

typedef struct {
  uint8_t b, g, r, a;
} BGRA32;

typedef struct {
	uint32_t dx, dy;
	BGRA32 *p_data;
} Pixelmap32;

typedef struct {
	int32_t x0, y0, x1, y1;
} Rectangle;

#pragma pack(pop)

Pixelmap32 *NewPixelmap32(uint32_t dx, uint32_t dy);

void DeletePixelmap32(Pixelmap32 **ppPm);

int ScalePixelmap32(Pixelmap32 *pDstPm, Rectangle  *pDstRc,
	Pixelmap32 *pSrcPm, Rectangle  *pSrcRc);

#ifdef __cplusplus
}
#endif

#endif // _Pixelmap32_h_
