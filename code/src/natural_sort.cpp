#include "wx/wx.h"
#include "wx/regex.h"

namespace    // enum, class and functions needed by wxCmpNatural().
{
    enum wxStringFragmentType
    {
        wxFRAGMENT_TYPE_EMPTY = 0,
        wxFRAGMENT_TYPE_ALPHA = 1,
        wxFRAGMENT_TYPE_DIGIT = 2
    };


    // ----------------------------------------------------------------------------
    // wxStringFragment
    // ----------------------------------------------------------------------------
    // 
    // Lightweight object returned by GetNaturalFragment().
    // Represents either a number, or a string which contains no numerical digits.
    class wxStringFragment
    {
    public:
        wxStringFragment()
            : type(wxFRAGMENT_TYPE_EMPTY)
        {}

        wxString              text;
        long                  value;
        wxStringFragmentType  type;
    };


    wxStringFragment GetFragment(wxString& text)
    {
        static const wxRegEx naturalNumeric(wxS("[0-9]+"));
        static const wxRegEx naturalAlpha(wxS("[^0-9]+"));

        size_t           digitStart  = 0;
        size_t           digitLength = 0;
        size_t           alphaStart  = 0;
        size_t           alphaLength = 0;
        wxStringFragment fragment;

        if ( text.empty() )
            return fragment;

        if ( naturalNumeric.Matches(text) )
        {
            naturalNumeric.GetMatch(&digitStart, &digitLength, 0);
        }

        if ( naturalAlpha.Matches(text) )
        {
            naturalAlpha.GetMatch(&alphaStart, &alphaLength, 0);
        }


        if ( alphaStart == 0 )
        {
            fragment.text = text.Mid(0, alphaLength);
            fragment.value = 0;
            fragment.type = wxFRAGMENT_TYPE_ALPHA;

            text.erase(0, alphaLength);
        }

        if ( digitStart == 0 )
        {
            fragment.text = text.Mid(0, digitLength);
            fragment.text.ToLong(&fragment.value);
            fragment.type = wxFRAGMENT_TYPE_DIGIT;

            text.erase(0, digitLength);
        }

        return fragment;
    }

    int CompareFragmentNatural(const wxStringFragment& lhs, const wxStringFragment& rhs)
    {
        if ( (lhs.type == wxFRAGMENT_TYPE_ALPHA) &&
             (rhs.type == wxFRAGMENT_TYPE_ALPHA) )
        {
            return lhs.text.CmpNoCase(rhs.text);
        }

        if ( (lhs.type == wxFRAGMENT_TYPE_DIGIT) &&
             (rhs.type == wxFRAGMENT_TYPE_DIGIT) )
        {
            if ( lhs.value == rhs.value )
            {
                return  0;
            }

            if ( lhs.value < rhs.value )
            {
                return -1;
            }

            if ( lhs.value > rhs.value )
            {
                return  1;
            }
        }

        if ( (lhs.type == wxFRAGMENT_TYPE_DIGIT) &&
             (rhs.type == wxFRAGMENT_TYPE_ALPHA) )
        {
            return -1;
        }

        if ( (lhs.type == wxFRAGMENT_TYPE_ALPHA) &&
             (rhs.type == wxFRAGMENT_TYPE_DIGIT) )
        {
            return 1;
        }

        if ( lhs.type == wxFRAGMENT_TYPE_EMPTY )
        {
            return -1;
        }

        if ( rhs.type == wxFRAGMENT_TYPE_EMPTY )
        {
            return 1;
        }

        return 0;
    }

} // unnamed namespace



// ----------------------------------------------------------------------------
// wxCmpNaturalNative
// ----------------------------------------------------------------------------
// 
int wxCMPFUNC_CONV wxCmpNatural(const wxString& s1, const wxString& s2)
{
    wxString lhs(s1);
    wxString rhs(s2);

    int comparison = 0;

    while ( (comparison == 0) && (!lhs.empty() || !rhs.empty()) )
    {
        wxStringFragment fragmentL = GetFragment(lhs);
        wxStringFragment fragmentR = GetFragment(rhs);
        comparison = CompareFragmentNatural(fragmentL, fragmentR);
    }

    return comparison;
}


// ----------------------------------------------------------------------------
// Declaration of StrCmpLogicalW()
// ----------------------------------------------------------------------------
// 
// In some distributions of MinGW32, this function is exported in the library,
// but not declared in shlwapi.h. Therefore we declare it here.
#if defined( __MINGW32_TOOLCHAIN__ )
    extern "C" __declspec(dllimport) int WINAPI StrCmpLogicalW(LPCWSTR psz1, LPCWSTR psz2);
#endif


// ----------------------------------------------------------------------------
// wxCmpNaturalNative
// ----------------------------------------------------------------------------
// 
// If a native version of Natural sort is available, then use that, otherwise
// use the wxWidgets version, wxCmpNatural(). 
int wxCMPFUNC_CONV wxCmpNaturalNative(const wxString& s1, const wxString& s2)
{
    return wxCmpNatural( s1, s2 );
}
