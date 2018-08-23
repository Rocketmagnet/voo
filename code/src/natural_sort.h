#ifndef NATURAL_SORT_H_INCLUDED
#define NATURAL_SORT_H_INCLUDED


WXDLLIMPEXP_BASE int wxCMPFUNC_CONV wxCmpNatural(      const wxString& s1, const wxString& s2);
WXDLLIMPEXP_BASE int wxCMPFUNC_CONV wxCmpNaturalNative(const wxString& s1, const wxString& s2);

inline int wxCMPFUNC_CONV wxNaturalStringSortAscending(const wxString& s1, const wxString& s2)
{
    return wxCmpNatural(s1, s2);
}

inline int wxCMPFUNC_CONV wxNaturalStringSortDescending(const wxString& s1, const wxString& s2)
{
    return wxCmpNatural(s2, s1);
}


#endif
