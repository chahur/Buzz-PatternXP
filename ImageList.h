
#pragma once

inline void CreateImageList(CImageList &il, PCTSTR pszName, int cx, int nGrow, COLORREF crMask) 
{ 
	CBitmap bitmap; 
	VERIFY(bitmap.LoadBitmap(pszName));
	BITMAP bm; 
	bitmap.GetBitmap( &bm ); 
	UINT flags = 0; 
	switch (bm.bmBitsPixel) 
	{ 
	case 4: flags = ILC_COLOR4; break; 
	case 8: flags = ILC_COLOR8; break; 
	case 16: flags = ILC_COLOR16; break; 
	case 24: flags = ILC_COLOR24; break; 
	case 32: flags = ILC_COLOR32; break; 
	default: flags = ILC_COLOR4; break;           
	} 

	if (!il.Create(cx,bm.bmHeight,flags|ILC_MASK,0,nGrow)) 
			return;

	il.Add(&bitmap,crMask); 
}
