/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <atlcoll.h>
#include <array>
#include "BaseClasses/wxutil.h"
#include "TextFile.h"
#include "SubtitleHelpers.h"

enum tmode { TIME, FRAME }; // the meaning of STSEntry::start/end

class STSStyle
{
public:
    CRect    marginRect;             // measured from the sides
    int      scrAlignment;           // 1 - 9: as on the numpad, 0: default
    int      borderStyle;            // 0: outline, 1: opaque box
    double   outlineWidthX, outlineWidthY;
    double   shadowDepthX, shadowDepthY;
    std::array<COLORREF, 4> colors;  // usually: {primary, secondary, outline/background, shadow}
    std::array<BYTE, 4> alpha;
    int      charSet;
    CString  fontName;
    double   fontSize;               // height
    double   fontScaleX, fontScaleY; // percent
    double   fontSpacing;            // +/- pixels
    LONG     fontWeight;
    int      fItalic;
    int      fUnderline;
    int      fStrikeOut;
    int      fBlur;
    double   fGaussianBlur;
    double   fontAngleZ, fontAngleX, fontAngleY;
    double   fontShiftX, fontShiftY;
    int      relativeTo;             // 0: window, 1: video, 2: undefined (~window)

    STSStyle();

    void SetDefault();

    bool operator == (const STSStyle& s) const;
    bool operator != (const STSStyle& s) const {
        return !(*this == s);
    };
    bool IsFontStyleEqual(const STSStyle& s) const;

    STSStyle& operator = (LOGFONT& lf);

    friend LOGFONTA& operator <<= (LOGFONTA& lfa, STSStyle& s);
    friend LOGFONTW& operator <<= (LOGFONTW& lfw, STSStyle& s);

    friend CString& operator <<= (CString& style, const STSStyle& s);
    friend STSStyle& operator <<= (STSStyle& s, const CString& style);
};

class CSTSStyleMap : public CAtlMap<CString, STSStyle*, CStringElementTraits<CString>>
{
public:
    CSTSStyleMap() {}
    virtual ~CSTSStyleMap() { Free(); }
    void Free();
};

struct STSEntry {
    CStringW str;
    bool fUnicode;
    CString style, actor, effect;
    CRect marginRect;
    int layer;
    int start, end;
    int readorder;
};

class STSSegment
{
public:
    int start, end;
    CAtlArray<int> subs;

    STSSegment() : start(0), end(0) {}
    STSSegment(int s, int e) {
        start = s;
        end = e;
    }
    STSSegment(const STSSegment& stss) {
        *this = stss;
    }
    STSSegment& operator = (const STSSegment& stss) {
        if (this != &stss) {
            start = stss.start;
            end = stss.end;
            subs.Copy(stss.subs);
        }
        return *this;
    }
};

class CSimpleTextSubtitle : public CAtlArray<STSEntry>
{
    friend class CSubtitleEditorDlg;

protected:
    CAtlArray<STSSegment> m_segments;
    virtual void OnChanged() {}

public:
    CString m_name;
    LCID m_lcid;
    Subtitle::SubType m_subtitleType;
    tmode m_mode;
    CTextFile::enc m_encoding;
    CString m_path;

    CSize m_dstScreenSize;
    int m_defaultWrapStyle;
    int m_collisions;
    bool m_fScaledBAS;

    bool m_fUsingAutoGeneratedDefaultStyle;

    CSTSStyleMap m_styles;

    enum EPARCompensationType {
        EPCTDisabled     = 0,
        EPCTDownscale    = 1,
        EPCTUpscale      = 2,
        EPCTAccurateSize = 3
    };

    EPARCompensationType m_ePARCompensationType;
    double m_dPARCompensation;

public:
    CSimpleTextSubtitle();
    virtual ~CSimpleTextSubtitle();

    virtual void Copy(CSimpleTextSubtitle& sts);
    virtual void Empty();

    void Sort(bool fRestoreReadorder = false);
    void CreateSegments();

    void Append(CSimpleTextSubtitle& sts, int timeoff = -1);

    bool Open(CString fn, int CharSet, CString name = _T(""), CString videoName = _T(""));
    bool Open(CTextFile* f, int CharSet, CString name);
    bool Open(BYTE* data, int len, int CharSet, CString name);
    bool SaveAs(CString fn, Subtitle::SubType type, double fps = -1, int delay = 0, CTextFile::enc = CTextFile::DEFAULT_ENCODING);

    void Add(CStringW str, bool fUnicode, int start, int end, CString style = _T("Default"), CString actor = _T(""), CString effect = _T(""), const CRect& marginRect = CRect(0, 0, 0, 0), int layer = 0, int readorder = -1);
    STSStyle* CreateDefaultStyle(int CharSet);
    void ChangeUnknownStylesToDefault();
    void AddStyle(CString name, STSStyle* style); // style will be stored and freed in Empty() later
    bool CopyStyles(const CSTSStyleMap& styles, bool fAppend = false);

    bool SetDefaultStyle(const STSStyle& s);
    bool GetDefaultStyle(STSStyle& s) const;

    void ConvertToTimeBased(double fps);
    void ConvertToFrameBased(double fps);

    int TranslateStart(int i, double fps);
    int TranslateEnd(int i, double fps);
    int SearchSub(int t, double fps);

    int TranslateSegmentStart(int i, double fps);
    int TranslateSegmentEnd(int i, double fps);
    const STSSegment* SearchSubs(int t, double fps, /*[out]*/ int* iSegment = nullptr, int* nSegments = nullptr);
    const STSSegment* GetSegment(int iSegment) {
        return iSegment >= 0 && iSegment < (int)m_segments.GetCount() ? &m_segments[iSegment] : nullptr;
    }

    STSStyle* GetStyle(int i);
    bool GetStyle(int i, STSStyle& stss);
    bool GetStyle(CString styleName, STSStyle& stss);
    int GetCharSet(int i);
    bool IsEntryUnicode(int i);
    void ConvertUnicode(int i, bool fUnicode);

    CStringA GetStrA(int i, bool fSSA = false);
    CStringW GetStrW(int i, bool fSSA = false);
    CStringW GetStrWA(int i, bool fSSA = false);

    void SetStr(int i, CStringA str, bool fUnicode /* ignored */);
    void SetStr(int i, CStringW str, bool fUnicode);
};

extern BYTE CharSetList[];
extern TCHAR* CharSetNames[];
extern int CharSetLen;

class CHtmlColorMap : public CAtlMap<CString, DWORD, CStringElementTraits<CString>>
{
public:
    CHtmlColorMap();
};

extern CHtmlColorMap g_colors;
