#pragma once

#include	<wxWebMap.h>
#include    <MapHtml.h>
#include    <list>

namespace iconic {
class wxWebMapImpl : public wxWebMap {
public:
    static wxWebMap* Create(wxWindow* parent, wxWindowID id, wxString const& basemapHtmlFileName, wxString const& dataDirectory,
                            const wxString& url, const wxPoint& pos, const wxSize& size, const wxString& backend, long style, const wxString& name);

protected:
    wxWebMapImpl(wxString const &basemapHtmlFileName, wxString const& dataDirectory);

    virtual wxWebView* GetWebView();

    virtual bool AddMapObject(wxMapObject const& o, wxString* result);

    wxWebView* cpWebView;
    MapHtmlPtr cpMap;
};
}