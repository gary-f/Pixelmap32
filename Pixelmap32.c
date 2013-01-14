/*
  Pixelmap32.c

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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "Pixelmap32.h"

#define DOWN_NBITS 11

/*--------------------------------------------------------------------------------------------------------------------*/
static int32_t RectangleDx(Rectangle *pRc)
{
	return (pRc->x1 + pRc->x0 + 1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
static int32_t RectangleDy(Rectangle *pRc)
{
	return (pRc->y1 + pRc->y0 + 1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
static void RectangleSetDx(Rectangle *pRc, int32_t dx)
{
	pRc->x1 = pRc->x0 + dx - 1;
}

/*--------------------------------------------------------------------------------------------------------------------*/
static void RectangleSetDy(Rectangle *pRc, int32_t dy)
{
	pRc->y1 = pRc->y0 + dy - 1;
}

/*--------------------------------------------------------------------------------------------------------------------*/
static int RectangleIsNull(Rectangle *pRc)
{
	if (pRc == NULL)
		return !0;

	if (pRc->x1 < pRc->x0 || pRc->y1 < pRc->y0)
		return !0;

	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
Pixelmap32 *NewPixelmap32(uint32_t dx, uint32_t dy)
{
	Pixelmap32 *pPm = (Pixelmap32*)calloc(1, sizeof(Pixelmap32));
	if (pPm != NULL)
	{
		pPm->dx = dx;
		pPm->dy = dy;
		if (dx > 0 && dy > 0) {
			pPm->p_data = malloc(dx * dy * sizeof(BGRA32));
			if (pPm->p_data == NULL)
			{
				free(pPm);
				pPm = NULL;
			}
		} else {
			pPm->p_data = NULL;
		}
	}
	return pPm;
}

/*--------------------------------------------------------------------------------------------------------------------*/
void DeletePixelmap32(Pixelmap32 **ppPm)
{
	if (ppPm != NULL)
	{
		Pixelmap32 *pPm = *ppPm;
		if (pPm != NULL)
		{
			if (pPm->p_data != NULL)
			{
				free(pPm->p_data);
				pPm->p_data = NULL;
			}
			free(pPm);
			*ppPm = NULL;
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/
static int Pixelmap32IsEmpty(Pixelmap32 *pPm)
{
	if (pPm == NULL)
		return !0;

	if (pPm->p_data == NULL)
		return !0;

	if (pPm->dx == 0|| pPm->dy == 0)
		return !0;

	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
static BGRA32 *GetPixelPtr(Pixelmap32 *pPm, uint32_t x0, uint32_t y0)
{
	return (pPm->p_data + x0 + (pPm->dx * y0));
}

//-----------------------------------------------------------------------------
static int ClipBlt
(
    Pixelmap32 *pDstPm,
    Rectangle  *pDstRc,
    Pixelmap32 *pSrcPm,
    Rectangle  *pSrcRc
)
{// ASSUMES ((lDstDx == lSrcDx) && (lDstDy == lSrcDy)) && (Pm's !Empty)
    int32_t lTmp;

    if (pDstRc->x0 < 0)
    {
        pSrcRc->x0 -= pDstRc->x0;
        pDstRc->x0 = 0;
    }
    if (pSrcRc->x0 < 0)
    {
        pDstRc->x0 -= pSrcRc->x0;
        pSrcRc->x0 = 0;
    }
    if (pDstRc->y0 < 0)
    {
        pSrcRc->y0 -= pDstRc->y0;
        pDstRc->y0 = 0;
    }
    if (pSrcRc->y0 < 0)
    {
        pDstRc->y0 -= pSrcRc->y0;
        pSrcRc->y0 = 0;
    }

    lTmp = ((pDstRc->x1 - pDstPm->dx) + 1);
    if (lTmp > 0)
    {
        pDstRc->x1 -= lTmp;
        pSrcRc->x1 -= lTmp;
    }
    lTmp = ((pSrcRc->x1 - pSrcPm->dx) + 1);
    if (lTmp > 0)
    {
        pSrcRc->x1 -= lTmp;
        pDstRc->x1 -= lTmp;
    }
    lTmp = ((pDstRc->y1 - pDstPm->dy) + 1);
    if (lTmp > 0)
    {
        pDstRc->y1 -= lTmp;
        pSrcRc->y1 -= lTmp;
    }
    lTmp = ((pSrcRc->y1 - pSrcPm->dy) + 1);
    if (lTmp > 0)
    {
        pSrcRc->y1 -= lTmp;
        pDstRc->y1 -= lTmp;
    }

    return (!RectangleIsNull(pDstRc) && !RectangleIsNull(pSrcRc));
}

//-----------------------------------------------------------------------------
static int ClipScaleBlt
(
    Pixelmap32 *pDstPm,
    Rectangle  *pDstRc,
    Pixelmap32 *pSrcPm,
    Rectangle  *pSrcRc
)
{// ASSUMES (Pm's !Empty)
    int32_t lDstDx = RectangleDx(pDstRc);
    int32_t lDstDy = RectangleDy(pDstRc);
    int32_t lSrcDx = RectangleDx(pSrcRc);
    int32_t lSrcDy = RectangleDy(pSrcRc);
    int32_t lTmp;

    if (pDstRc->x0 < 0)
    {
        pSrcRc->x0 -= ((pDstRc->x0 * lSrcDx) / lDstDx);
        pDstRc->x0 = 0;
    }
    if (pSrcRc->x0 < 0)
    {
        pDstRc->x0 -= ((pSrcRc->x0 * lDstDx) / lSrcDx);
        pSrcRc->x0 = 0;
    }
    if (pDstRc->y0 < 0)
    {
        pSrcRc->y0 -= ((pDstRc->y0 * lSrcDy) / lDstDy);
        pDstRc->y0 = 0;
    }
    if (pSrcRc->y0 < 0)
    {
        pDstRc->y0 -= ((pSrcRc->y0 * lDstDy) / lSrcDy);
        pSrcRc->y0 = 0;
    }

    lTmp = ((pDstRc->x1 - pDstPm->dx) + 1);
    if (lTmp > 0)
    {
        pDstRc->x1 -= lTmp;
        pSrcRc->x1 -= ((lTmp * lSrcDx) / lDstDx);
    }
    lTmp = ((pSrcRc->x1 - pSrcPm->dx) + 1);
    if (lTmp > 0)
    {
        pSrcRc->x1 -= lTmp;
        pDstRc->x1 -= ((lTmp * lDstDx) / lSrcDx);
    }
    lTmp = ((pDstRc->y1 - pDstPm->dy) + 1);
    if (lTmp > 0)
    {
        pDstRc->y1 -= lTmp;
        pSrcRc->y1 -= ((lTmp * lSrcDy) / lDstDy);
    }
    lTmp = ((pSrcRc->y1 - pSrcPm->dy) + 1);
    if (lTmp > 0)
    {
        pSrcRc->y1 -= lTmp;
        pDstRc->y1 -= ((lTmp * lDstDy) / lSrcDy);
    }

    return (!RectangleIsNull(pDstRc) && !RectangleIsNull(pSrcRc));
}

/*--------------------------------------------------------------------------------------------------------------------*/
static void ScaleUpX
(
    Pixelmap32 *pDstPm,
    Rectangle  *pDstRc,
    Pixelmap32 *pSrcPm,
    Rectangle  *pSrcRc
)
// assumes:
//  all arguments point to valid data
//  prcDst is within pDstPm
//  prcSrc is within this pixelmap
//  prcSrc->dy == prcDst->dy
{
// TODO:
//  only play DDA once, store Acc's in an array then rip
    int32_t lDstDx = RectangleDx(pDstRc);
    int32_t lDstDy = RectangleDy(pDstRc);
    int32_t lSrcDx = RectangleDx(pSrcRc);

    BGRA32 *pDstLine = GetPixelPtr(pDstPm, pDstRc->x0, pDstRc->y0);
    BGRA32 *pSrcLine = GetPixelPtr(pSrcPm, pSrcRc->x0, pSrcRc->y0);

    uint32_t ulInc = (lSrcDx - 1);
    ulInc = ((ulInc * 4096) / (lDstDx - 1));

    int32_t lYCnt = lDstDy;
    while (lYCnt--)
    {
        BGRA32 *pDst = pDstLine;
        BGRA32 *pSrc = pSrcLine;
        uint32_t ulAcc = 0;
        int32_t lXCnt = lDstDx;
        while (lXCnt--)
        {
            if (ulAcc == 0)
            {
                *pDst++ = *pSrc;
            }
            else
            {
                BGRA32 *pSrc1 = (pSrc + 1);
                uint32_t ulAcn = (4096 - ulAcc);
                pDst->b = (uint8_t)(((pSrc->b * ulAcn) + (pSrc1->b * ulAcc)) >> 12);
                pDst->g = (uint8_t)(((pSrc->g * ulAcn) + (pSrc1->g * ulAcc)) >> 12);
                pDst->r = (uint8_t)(((pSrc->r * ulAcn) + (pSrc1->r * ulAcc)) >> 12);
                pDst->a = (uint8_t)(((pSrc->a * ulAcn) + (pSrc1->a * ulAcc)) >> 12);
                pDst++;
            }

            ulAcc += ulInc;
            if (ulAcc & 4096)
            {
                ulAcc &= 4095;
                pSrc++;
            }
        }
        pDstLine += pDstPm->dx;
        pSrcLine += pSrcPm->dx;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
static void ScaleUpY
(
    Pixelmap32 *pDstPm,
    Rectangle  *pDstRc,
    Pixelmap32 *pSrcPm,
    Rectangle  *pSrcRc
)
{
    int32_t lDstDx = RectangleDx(pDstRc);
    int32_t lDstDy = RectangleDy(pDstRc);
    int32_t lSrcDy = RectangleDy(pSrcRc);

    BGRA32 *pDstLine = GetPixelPtr(pDstPm, pDstRc->x0, pDstRc->y1);
    BGRA32 *pSrcLine = GetPixelPtr(pSrcPm, pSrcRc->x0, pSrcRc->y1);

    uint32_t ulInc = (lSrcDy - 1);
    ulInc = ((ulInc * 4096) / (lDstDy - 1));

    uint32_t ulAcc = 0;
    int32_t lYCnt = lDstDy;
    while (lYCnt--)
    {
        if (ulAcc == 0)
        {
        	memcpy(pDstLine, pSrcLine, (lDstDx << 2));
        }
        else
        {
            BGRA32 *pDst = pDstLine;
            BGRA32 *pSrc = pSrcLine;
            BGRA32 *pSrc1 = (pSrc - pSrcPm->dx);

            uint32_t ulAcn = (4096 - ulAcc);

        	int32_t lXCnt = lDstDx;
            while (lXCnt--)
            {
                pDst->b = (uint8_t)(((pSrc->b * ulAcn) + (pSrc1->b * ulAcc)) >> 12);
                pDst->g = (uint8_t)(((pSrc->g * ulAcn) + (pSrc1->g * ulAcc)) >> 12);
                pDst->r = (uint8_t)(((pSrc->r * ulAcn) + (pSrc1->r * ulAcc)) >> 12);
                pDst->a = (uint8_t)(((pSrc->a * ulAcn) + (pSrc1->a * ulAcc)) >> 12);
                pDst++;
                pSrc++;
                pSrc1++;
            }
        }

        ulAcc += ulInc;
        if (ulAcc & 4096)
        {
            ulAcc &= 4095;
            pSrcLine -= pSrcPm->dx;
        }

        pDstLine -= pDstPm->dx;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
static void ScaleDownY
(
    Pixelmap32 *pDstPm,
    Rectangle  *pDstRc,
    Pixelmap32 *pSrcPm,
    Rectangle  *pSrcRc
)
// assumes:
//  all arguments point to valid data
//  prcDst is within pDstPm
//  prcSrc is within this pixelmap
//  prcSrc->dx == prcDst->dx
{
    int32_t lDstDx = RectangleDx(pDstRc);
    int32_t lDstDy = RectangleDy(pDstRc);
    int32_t lSrcDy = RectangleDy(pSrcRc);

    BGRA32 *pDstLine = GetPixelPtr(pDstPm, pDstRc->x0, pDstRc->y0);
    BGRA32 *pSrcLine = GetPixelPtr(pSrcPm, pSrcRc->x0, pSrcRc->y0);

    uint32_t ulMax = lDstDy;
    uint32_t ulInc = lSrcDy;
    ulInc = ((ulInc * (1 << DOWN_NBITS)) / ulMax);
	ulMax = 1 << DOWN_NBITS;

    uint32_t ulB = 0;
    uint32_t ulG = 0;
    uint32_t ulR = 0;
    uint32_t ulA = 0;
    uint32_t ulD = 0;

    BGRA32 *pDst;
    BGRA32 *pSrc;

    uint32_t ulAcc;
    uint32_t ulAcn;
    int32_t lYCnt;

    int32_t lXCnt = lDstDx;
    while (lXCnt--)
    {
        pSrc = pSrcLine;
        pDst = pDstLine;
        ulAcc = 0;
        lYCnt = lDstDy;
        while (lYCnt--)
        {
            if (ulAcc != 0)
            {
                ulAcn = (ulMax - ulAcc);
                ulB += (pSrc->b * ulAcn);
                ulG += (pSrc->g * ulAcn);
                ulR += (pSrc->r * ulAcn);
                ulA += (pSrc->a * ulAcn);
                ulD += ulAcn;
                ulAcc -= ulMax;
                pSrc += pSrcPm->dx;
            }
            ulAcc += ulInc;
            while (ulAcc >= ulMax)
            {
                ulB += ((uint32_t)pSrc->b << DOWN_NBITS);
                ulG += ((uint32_t)pSrc->g << DOWN_NBITS);
                ulR += ((uint32_t)pSrc->r << DOWN_NBITS);
                ulA += ((uint32_t)pSrc->a << DOWN_NBITS);
                ulD += ulMax;
                ulAcc -= ulMax;
                pSrc += pSrcPm->dx;
            }
            if (ulAcc != 0)
            {
                ulB += (pSrc->b * ulAcc);
                ulG += (pSrc->g * ulAcc);
                ulR += (pSrc->r * ulAcc);
                ulA += (pSrc->a * ulAcc);
                ulD += ulAcc;
            }

            pDst->b = (uint8_t)((ulB + 1024) / ulD);
            pDst->g = (uint8_t)((ulG + 1024) / ulD);
            pDst->r = (uint8_t)((ulR + 1024) / ulD);
            pDst->a = (uint8_t)((ulA + 1024) / ulD);
            pDst += pDstPm->dx;

            ulB = 0;
            ulG = 0;
            ulR = 0;
            ulA = 0;
            ulD = 0;
        }
        pSrcLine++;
        pDstLine++;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
static void ScaleDownX
(
    Pixelmap32 *pDstPm,
    Rectangle  *pDstRc,
    Pixelmap32 *pSrcPm,
    Rectangle  *pSrcRc
)
// assumes:
//  all arguments point to valid data
//  prcDst is within pDstPm
//  prcSrc is within this pixelmap
//  prcSrc->dy == prcDst->dy
{
    int32_t lDstDx = RectangleDx(pDstRc);
    int32_t lDstDy = RectangleDy(pDstRc);
    int32_t lSrcDx = RectangleDx(pSrcRc);

    BGRA32 *pDstLine = GetPixelPtr(pDstPm, pDstRc->x0, pDstRc->y0);
    BGRA32 *pSrcLine = GetPixelPtr(pSrcPm, pSrcRc->x0, pSrcRc->y0);

    uint32_t ulMax = lDstDx;
    uint32_t ulInc = lSrcDx;
    ulInc = ((ulInc * (1 << DOWN_NBITS)) / ulMax);
	ulMax = 1 << DOWN_NBITS;

    int32_t lYCnt = lDstDy;
    while (lYCnt--)
    {
        BGRA32 *pDst = pDstLine;
        BGRA32 *pSrc = pSrcLine;

        uint32_t ulB = 0;
        uint32_t ulG = 0;
        uint32_t ulR = 0;
        uint32_t ulA = 0;
        uint32_t ulD = 0;

        uint32_t ulAcc = 0;
        int32_t lXCnt = lDstDx;
        while (lXCnt--)
        {
            if (ulAcc != 0)
            {
                uint32_t ulAcn = (ulMax - ulAcc);
                ulB += (pSrc->b * ulAcn);
                ulG += (pSrc->g * ulAcn);
                ulR += (pSrc->r * ulAcn);
                ulA += (pSrc->a * ulAcn);
                ulD += ulAcn;
                ulAcc -= ulMax;
                pSrc++;
            }
            ulAcc += ulInc;
            while (ulAcc >= ulMax)
            {
                ulB += ((uint32_t)pSrc->b << DOWN_NBITS);
                ulG += ((uint32_t)pSrc->g << DOWN_NBITS);
                ulR += ((uint32_t)pSrc->r << DOWN_NBITS);
                ulA += ((uint32_t)pSrc->a << DOWN_NBITS);
                ulD += ulMax;
                ulAcc -= ulMax;
                pSrc++;
            }
            if (ulAcc != 0)
            {
                ulB += (pSrc->b * ulAcc);
                ulG += (pSrc->g * ulAcc);
                ulR += (pSrc->r * ulAcc);
                ulA += (pSrc->a * ulAcc);
                ulD += ulAcc;
            }
            pDst->b = (uint8_t)((ulB + (ulMax>>1)) / ulD);
            pDst->g = (uint8_t)((ulG + (ulMax>>1)) / ulD);
            pDst->r = (uint8_t)((ulR + (ulMax>>1)) / ulD);
            pDst->a = (uint8_t)((ulA + (ulMax>>1)) / ulD);
            pDst++;

            ulB = 0;
            ulG = 0;
            ulR = 0;
            ulA = 0;
            ulD = 0;
        }
        pDstLine += pDstPm->dx;
        pSrcLine += pSrcPm->dx;
    }
}

//-----------------------------------------------------------------------------
int ScalePixelmap32
(
    Pixelmap32 *pDstPm,
    Rectangle  *pDstRc,
    Pixelmap32 *pSrcPm,
    Rectangle  *pSrcRc
)
{
    if (Pixelmap32IsEmpty(pDstPm) || Pixelmap32IsEmpty(pSrcPm) ||
        RectangleIsNull(pDstRc) || RectangleIsNull(pSrcRc))
        return !0; // true

    Rectangle rcDstC = *pDstRc; // clipped dst rect
    Rectangle rcSrcC = *pSrcRc; // clipped src rect

    if ((RectangleDx(&rcDstC) == RectangleDx(&rcSrcC)) &&
        (RectangleDy(&rcDstC) == RectangleDy(&rcSrcC)))
    {// NOT SCALING
        if (ClipBlt(pDstPm, &rcDstC, pSrcPm, &rcSrcC))
        {
            int32_t lDstDx = RectangleDx(&rcDstC);
            int32_t lDstDy = RectangleDy(&rcDstC);

            BGRA32* pSrc = GetPixelPtr(pSrcPm, rcSrcC.x0, rcSrcC.y0);
            BGRA32* pDst = GetPixelPtr(pDstPm, rcDstC.x0, rcDstC.y0);

            int32_t lSrcLineAdv = (pSrcPm->dx - lDstDx);
            int32_t lDstLineAdv = (pDstPm->dx - lDstDx);

            int32_t lYCnt = lDstDy;
            while (lYCnt--)
            {
                int32_t lXCnt = lDstDx;
                while (lXCnt--)
                {
                    *pDst++ = *pSrc++;
                }
                pSrc += lSrcLineAdv;
                pDst += lDstLineAdv;
            }
        }
        return !0; // true
    }
    else
    {// SCALING
        if (ClipScaleBlt(pDstPm, &rcDstC, pSrcPm, &rcSrcC))
        {
            if ((RectangleDx(&rcDstC) > RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) > RectangleDy(&rcSrcC)))
            {
                Rectangle rcTmp = rcDstC;
                Pixelmap32 *pTmpPm;
                RectangleSetDy(&rcTmp, RectangleDy(&rcSrcC));

                pTmpPm = NewPixelmap32(RectangleDx(&rcTmp), RectangleDy(&rcTmp));

                ScaleUpX(pTmpPm, &rcTmp, pSrcPm, &rcSrcC);
                ScaleUpY(pDstPm, &rcDstC, pTmpPm, &rcTmp);
                DeletePixelmap32(&pTmpPm);
                return !0; // true
            }
            else
            if ((RectangleDx(&rcDstC) < RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) < RectangleDy(&rcSrcC)))
            {
                Rectangle rcTmp = {0, 0, 0, 0};
                Pixelmap32 *pTmpPm;
				RectangleSetDx(&rcTmp, RectangleDx(&rcDstC));
				RectangleSetDy(&rcTmp, RectangleDy(&rcSrcC));

                pTmpPm = NewPixelmap32(RectangleDx(&rcTmp), RectangleDy(&rcTmp));

                ScaleDownX(pTmpPm, &rcTmp, pSrcPm, &rcSrcC);
                ScaleDownY(pDstPm, &rcDstC, pTmpPm, &rcTmp);
                DeletePixelmap32(&pTmpPm);
                return !0; // true
            }
            else
            if ((RectangleDx(&rcDstC) < RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) > RectangleDy(&rcSrcC)))
            {
                Rectangle rcTmp = {0, 0, 0, 0};
                Pixelmap32 *pTmpPm;
                RectangleSetDx(&rcTmp, RectangleDx(&rcDstC));
				RectangleSetDy(&rcTmp, RectangleDy(&rcSrcC));

                pTmpPm = NewPixelmap32(RectangleDx(&rcTmp), RectangleDy(&rcTmp));

                ScaleDownX(pTmpPm, &rcTmp, pSrcPm, &rcSrcC);
				ScaleUpY(pDstPm, &rcDstC, pTmpPm, &rcTmp);
                DeletePixelmap32(&pTmpPm);
                return !0; // true
            }
            else
            if ((RectangleDx(&rcDstC) > RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) < RectangleDy(&rcSrcC)))
            {
                Rectangle rcTmp = {0, 0, 0, 0};
                Pixelmap32 *pTmpPm;
				RectangleSetDx(&rcTmp, RectangleDx(&rcSrcC));
				RectangleSetDy(&rcTmp, RectangleDy(&rcDstC));

                pTmpPm = NewPixelmap32(RectangleDx(&rcTmp), RectangleDy(&rcTmp));

				ScaleDownY(pTmpPm, &rcTmp, pSrcPm, &rcSrcC);
                ScaleUpX(pDstPm, &rcDstC, pTmpPm, &rcTmp);
                DeletePixelmap32(&pTmpPm);
                return !0; // true
            }
            else
            if ((RectangleDx(&rcDstC) >  RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) == RectangleDy(&rcSrcC)))
            {
                ScaleUpX(pDstPm, &rcDstC, pSrcPm, &rcSrcC);
                return !0; // true
            }
            else
            if ((RectangleDx(&rcDstC) <  RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) == RectangleDy(&rcSrcC)))
            {
				ScaleDownX(pDstPm, &rcDstC, pSrcPm, &rcSrcC);
                return !0; // true
            }
            else
            if ((RectangleDx(&rcDstC) == RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) >  RectangleDy(&rcSrcC)))
            {
                ScaleUpY(pDstPm, &rcDstC, pSrcPm, &rcSrcC);
                return !0; // true
            }
            else
            if ((RectangleDx(&rcDstC) == RectangleDx(&rcSrcC)) &&
                (RectangleDy(&rcDstC) <  RectangleDy(&rcSrcC)))
            {
				ScaleDownY(pDstPm, &rcDstC, pSrcPm, &rcSrcC);
                return !0; // true
            }
            else
            {
                // IMPLEMENT ME?
                return 0; // false
            }
        }
    }

	return 0; // false
}

// EOF
