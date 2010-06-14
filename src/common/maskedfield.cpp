/////////////////////////////////////////////////////////////////////////////
// Name:        commun/maskedfield.cpp
// Purpose:
// Author:      Julien Weinzorn
// Id:          $Id: ????????????????????$
// Copyright:   (c) 2010 Julien Weinzorn
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"


#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <stdio.h>
#endif

#include "wx/maskedfield.h"

wxMaskedField::wxMaskedField()
{
}

wxMaskedField::wxMaskedField(const wxMaskedField& maskedField)
{
    m_mask         = maskedField.GetMask();
    m_formatCodes  = maskedField.GetFormatCodes();
    m_autoSelect   = maskedField.IsAutoSelect();
    m_decimalPoint = maskedField.GetDecimalPoint();
    m_useParensForNegatives = maskedField.IsParensForNegatives();
}

wxMaskedField::wxMaskedField( const wxString& mask
                 , const wxString& formatCodes
                 , const wxString& defaultValue
                 , const wxArrayString& choices
                 , const bool  autoSelect
                 , const wxChar groupChar, const wxChar decimalPoint
                 , const bool useParensForNegatives)
{
    unsigned int it;

    m_mask         = mask;
    m_formatCodes  = formatCodes;
    m_autoSelect   = autoSelect;
    m_groupChar    = groupChar;
    m_decimalPoint = decimalPoint;
    m_useParensForNegatives = useParensForNegatives;
    
    if(IsValid(defaultValue))
    {
        m_defaultValue = defaultValue;
    }
    else
    {
        m_defaultValue = wxT(" ");
    }
    
    for(it = 0; it < choices.size(); it++)
    {
        if(IsValid(choices[it]))
        {
            m_choices.Add(choices[it]);
        }
    }
}

bool wxMaskedField::IsEmpty(const wxString& string) const
{
    return string.Cmp(m_defaultValue) == 0;
}

bool wxMaskedField::IsNumber(const wxChar character) const
{
    return character >= '0' && character <= '9';
}

bool wxMaskedField::IsLowerCase(const wxChar character)const
{
    return character >= 'a' && character <= 'z';
}

bool wxMaskedField::IsUpperCase(const wxChar character) const
{
    return character >= 'A' && character <= 'Z';
}

bool wxMaskedField::IsPunctuation(const wxChar character) const
{
    return character == ',' || character == '?' || character == ';' 
        || character == '.' || character == ':' || character == '!';
}

wxString wxMaskedField::ApplyFormatCodes(const wxString& string)
{
    wxString res;
    unsigned int it;
    unsigned int itMask;
    bool tmp = false; //FIXME change name

    if(string.Len() > m_mask.Len())
        return wxEmptyString;

    for(it = 0, itMask = 0; itMask < m_mask.Len(); it++, itMask++)
    {

        if(IsCharValid(m_mask[itMask], string[it]))
        {

            res << string[it];
        }
        else if(m_mask[itMask] == '\\' && it != m_mask.Len() - 1)
        {
            if(string[it] == m_mask[itMask + 1])
                res << string[it] ;

            itMask++;
        }
        else
        {
            if(m_formatCodes.Contains(wxT("!")))
            {
                if(IsLowerCase(string[it]) && (m_mask[itMask] == 'A'
                   || m_mask[itMask] == 'N' || m_mask[itMask] == 'C' 
                   || m_mask[itMask] == 'X' || m_mask[itMask] == '*'))
                {
                    res << string.SubString(it, it).Upper();
                    tmp = true;
                }
            }
            else if(m_formatCodes.Contains(wxT("^")))
            {
                if(IsUpperCase(string[it]) && (m_mask[itMask] == 'a'
                   || m_mask[itMask] == 'N' || m_mask[itMask] == 'C' 
                   || m_mask[itMask] == 'X' || m_mask[itMask] == '*'))
                {
                    res << string.SubString(it, it).Lower();
                    tmp = true;
                }
            }
            
            
            if(m_formatCodes.Contains(wxT("_")) && tmp == false)
            {
                res << wxT(" ");
                it --;
            }

            tmp = false;
            
        }
    }
       
   
    for(it = 0; it < string.Len(); it++)
    {
        if(!res.Contains(string[it]))
        {
            if((m_formatCodes.Contains(wxT("^")) || m_formatCodes.Contains(wxT("!")) ) && ( res.Contains(string.SubString(it,it).Upper()) || res.Contains(string.SubString(it,it).Lower() )))

            {
            }
            else
            {
                it = string.Len() +1;
                res = wxEmptyString;
            }
        }
    }
    
    return res;
}


bool wxMaskedField::IsCharValid(const wxChar maskChar, const wxChar character) const
{
    bool res;
    switch(maskChar)
    {
        case '#':
            res = IsNumber(character);
        break;
        case 'N':
            res = IsNumber(character) || IsLowerCase(character) || IsUpperCase(character);
        break;
        case 'A':
            res = IsUpperCase(character);
        break;
        case 'a':
            res = IsLowerCase(character);
        break;
        case 'C':
            res = IsLowerCase(character) || IsUpperCase(character);
        break;
        case 'X':
            res = IsNumber(character) || IsLowerCase(character) || IsUpperCase(character)
                  || IsPunctuation(character);
        break;
        case '&':
            res = IsPunctuation(character);
        break;
        case '*':
            res = true;
        break;
        default:
            if(maskChar == character)
                res = true;
            else
                res = false;
    }

    return res;
}


bool wxMaskedField::IsValid(const wxString& string) const
{
    unsigned int it;
    unsigned int itMask;
    bool res = true;

    if(string.Len() > m_mask.Len() ||( string.Len() == 0 && m_mask.Len() !=0))
    {
        res = false;
    }

    for(it = 0, itMask = 0; itMask < m_mask.Len() && res && it < string.Len(); it++, itMask++)
    {
        if(m_mask[itMask] == '\\' && it != m_mask.Len() - 1)
        {
            if(string[it] != m_mask[itMask + 1])
                res = false;

            itMask++;
        }
        else if(!IsCharValid(m_mask[itMask], string[it]))
        {
            if(m_formatCodes.Contains('_'))
            {
                if(string[it] != ' ')
                {
                    res = false;
                }
            }
            else
            {
                res = false;
            }
        }
    }

    if(it != string.Len())
        res = false;
    
    return res;
    
}

bool wxMaskedField::AddChoice(const wxString& choice)
{
    bool res = true;
    if(IsValid(choice))
        m_choices.Add(choice);
    else
        res = false;

    return res;
}

bool wxMaskedField::AddChoices(const wxArrayString& choices)
{
    bool res = true;
    unsigned int it;

    for(it = 0; it < choices.size() && res; it++)
    {
        if(!IsValid(choices[it]))
            res = false;
    }

    for(it = 0; it < choices.size() && res; it++)
    {
        m_choices.Add(choices[it]);
    }

    return res;
}

void wxMaskedField::SetMask(const wxString& mask)
{
    m_mask = mask;
    unsigned int it;

    if(!IsValid(m_defaultValue))
        m_defaultValue = wxDEFAULT_VALUE;

    for(it = 0; it < m_choices.Count(); it++)
    {
        if(!IsValid(m_choices[it]))
        {
            m_choices.RemoveAt(it);
            it --;
        }
    }

}

wxString wxMaskedField::GetMask() const
{
    return m_mask;
}

wxString wxMaskedField::GetFormatCodes() const
{
    return m_formatCodes;
}

wxUniChar wxMaskedField::GetGroupChar() const
{
    return m_groupChar;
}

wxUniChar wxMaskedField::GetDecimalPoint() const
{
    return m_decimalPoint;
}

wxString wxMaskedField::GetDefaultValue() const
{
    return m_defaultValue;
}

wxArrayString wxMaskedField::GetChoices() const
{
    return m_choices;    
}

bool wxMaskedField::IsParensForNegatives() const
{
    return m_useParensForNegatives;
}

bool wxMaskedField::IsAutoSelect() const
{
    return m_autoSelect;
}



wxString wxMaskedField::GetPlainValue(const wxString& string)
{
    unsigned int it;
    unsigned int itMask;
    wxString res;
    
    if(!IsValid(string)||( string.Len() == 0 && m_mask.Len() !=0))
    {
        res = wxEmptyString;
    }
    else
    {
        for(it = 0, itMask = 0; itMask < m_mask.Len(); it++, itMask++)
        {
            if(m_mask[itMask] == '\\')
            {
                itMask++;
            }
            else if(string[it] != m_mask[it])
            {
                res << string[itMask];

                
            }
            else
            {
                if(m_mask[it] == 'C' || m_mask[it] == 'A' || m_mask[it] == 'a' ||
                   m_mask[it] == 'X' || m_mask[it] == '&' || m_mask[it] == '*' ||
                   m_mask[it] == 'N')
                {
                    res << string[itMask];
                }
            }
        }
    }
    return res;
}