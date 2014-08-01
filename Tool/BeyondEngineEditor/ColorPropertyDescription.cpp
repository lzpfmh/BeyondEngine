#include "stdafx.h"
#include "ColorPropertyDescription.h"
#include "Utility/BeatsUtility/StringHelper.h"
#include "Utility/BeatsUtility/Serializer.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

CColorPropertyDescription::CColorPropertyDescription(CSerializer* pSerializer)
    : super(eRPT_UInt)
{
    size_t value = 0;
    unsigned char r,g,b,a;
    if (pSerializer != NULL)
    {
        (*pSerializer) >> r;
        (*pSerializer) >> g;
        (*pSerializer) >> b;
        (*pSerializer) >> a;
        value = (r << 24) + (g << 16) + (b << 8) + a;
    }
    InitializeValue(value);
}

CColorPropertyDescription::CColorPropertyDescription(const CColorPropertyDescription& rRef)
    : super(rRef)
{
    size_t value = 0;
    InitializeValue(value);
}

CColorPropertyDescription::~CColorPropertyDescription()
{
    DestroyValue<size_t>();
}

bool CColorPropertyDescription::AnalyseUIParameterImpl(const std::vector<TString>& result)
{
    std::vector<TString> cache;
    for (size_t i = 0; i < result.size(); ++i)
    {
        cache.clear();
        CStringHelper::GetInstance()->SplitString(result[i].c_str(), PROPERTY_KEYWORD_SPLIT_STR, cache);
        BEATS_ASSERT(cache.size() == 2);
        if (_tcsicmp(cache[0].c_str(), UIParameterAttrStr[eUIPAT_DefaultValue]) == 0)
        {
            size_t uValue = 0;
            GetValueByTChar(cache[1].c_str(), &uValue);
            unsigned char r = uValue >> 24;
            unsigned char g = (uValue >> 16) & 0x000000FF;
            unsigned char b = (uValue >> 8) & 0x000000FF;
            unsigned char a = uValue & 0x000000FF;
            wxColour color(r, g, b, a);
            wxVariant var(color);
            SetValue(var, true);
            uValue = var.GetULongLong().GetLo();
            SetValueWithType(&uValue, eVT_DefaultValue);
        }
        else
        {
            BEATS_ASSERT(false, _T("Unknown parameter for UInt property %s"), cache[0].c_str());
        }
    }
    return true;
}

wxPGProperty* CColorPropertyDescription::CreateWxProperty()
{
    size_t uColorValue = *(size_t*)m_valueArray[eVT_CurrentValue];
    unsigned char r = uColorValue >> 24;
    unsigned char g = (uColorValue >> 16) & 0x000000FF;
    unsigned char b = (uColorValue >> 8) & 0x000000FF;
    unsigned char a = uColorValue & 0x000000FF;
    wxColour colorVaule(r, g, b, a);
    wxPGProperty* pProperty = new wxColourProperty(wxPG_LABEL, wxPG_LABEL, colorVaule);
    pProperty->SetAttribute("HasAlpha", true);
    pProperty->SetClientData(this);
    wxVariant var((wxLongLong)(*(size_t*)m_valueArray[eVT_CurrentValue]));
    pProperty->SetDefaultValue(var);
    pProperty->SetModifiedStatus(!IsDataSame(true));

    return pProperty;
}

void CColorPropertyDescription::SetValue( wxVariant& value, bool bSaveValue /*= true*/ )
{
    wxColor colourValue;
    colourValue << value ;
    size_t uNewValue = (colourValue.Red() << 24) + (colourValue.Green() << 16) + (colourValue.Blue() << 8) + colourValue.Alpha();
    SetValueWithType(&uNewValue, eVT_CurrentValue);
    if (bSaveValue)
    {
        SetValueWithType(&uNewValue, eVT_SavedValue);
    }
}

bool CColorPropertyDescription::CopyValue(void* pSourceValue, void* pTargetValue)
{
    bool bRet = *(size_t*)pTargetValue != *(size_t*)pSourceValue;
    if (bRet)
    {
        *(size_t*)pTargetValue = *(size_t*)pSourceValue;
    }
    return bRet;
}

bool CColorPropertyDescription::IsDataSame( bool bWithDefaultOrXML )
{
    bool bRet = *(size_t*)m_valueArray[(bWithDefaultOrXML ? eVT_DefaultValue : eVT_SavedValue)] == *(size_t*)m_valueArray[eVT_CurrentValue];
    return bRet;
}

CPropertyDescriptionBase* CColorPropertyDescription::CreateNewInstance()
{
    CPropertyDescriptionBase* pNewProperty = new CColorPropertyDescription(*this);
    return pNewProperty;
}

void CColorPropertyDescription::GetValueAsChar( EValueType type, char* pOut ) const
{
    size_t iValue = *(size_t*)m_valueArray[type];
    sprintf(pOut, "0x%x", iValue);
}

bool CColorPropertyDescription::GetValueByTChar(const TCHAR* pIn, void* pOutValue)
{
    TCHAR* pEndChar = NULL;
    _set_errno(0);
    size_t uValue = _tcstoul(pIn, &pEndChar, 16);
    BEATS_ASSERT(_tcslen(pEndChar) == 0, _T("Read uint from string %s error, stop at %s"), pIn, pEndChar);
    BEATS_ASSERT(errno == 0, _T("Call _tcstoul failed! string %s radix: 16"), pIn);
    *(size_t*)pOutValue = uValue;
    return true;
}

void CColorPropertyDescription::Serialize(CSerializer& serializer, EValueType eValueType /*= eVT_SavedValue*/)
{
    CColor color(*(size_t*)m_valueArray[eValueType]);
    serializer << color;
}

void CColorPropertyDescription::Deserialize(CSerializer& serializer, EValueType eValueType /*= eVT_CurrentValue*/)
{
    CColor color(*(size_t*)m_valueArray[eValueType]);
    serializer >> color;
}