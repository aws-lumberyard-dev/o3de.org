/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#include "CrySystem_precompiled.h"
#include "CmdLine.h"


void CCmdLine::PushCommand(const string& sCommand, const string& sParameter)
{
    if (sCommand.empty())
    {
        return;
    }

    ECmdLineArgType type = eCLAT_Normal;
    const char* szCommand = sCommand.c_str();

    if (sCommand[0] == '-')
    {
        type = eCLAT_Pre;
        ++szCommand;

        // Handle cmd line parameters that use -- properly
        if (szCommand[0] == '-')
        {
            ++szCommand;
        }
    }
    else if (sCommand[0] == '+')
    {
        type = eCLAT_Post;
        ++szCommand;
    }

    m_args.push_back(CCmdLineArg(szCommand, sParameter.c_str(), type));
}

CCmdLine::CCmdLine(const char* commandLine)
{
    m_sCmdLine = commandLine;

    char* src = (char*)commandLine;

    string command, parameter;

    for (;; )
    {
        if (*src == '\0')
        {
            break;
        }

        string arg = Next(src);

        if (m_args.empty())
        {
            // this is the filename, convert backslash to forward slash
            arg.replace('\\', '/');
            m_args.push_back(CCmdLineArg("filename", arg.c_str(), eCLAT_Executable));
        }
        else
        {
            bool bSecondCharIsNumber = false;

            if (arg[0] && arg[1] >= '0' && arg[1] <= '9')
            {
                bSecondCharIsNumber = true;
            }

            if ((arg[0] == '-' && !bSecondCharIsNumber)
                || (arg[0] == '+' && !bSecondCharIsNumber)
                || command.empty())                                             // separator or first parameter
            {
                PushCommand(command, parameter);

                command = arg;
                parameter = "";
            }
            else
            {
                if (parameter.empty())
                {
                    parameter = arg;
                }
                else
                {
                    parameter += string(" ") + arg;
                }
            }
        }
    }

    PushCommand(command, parameter);
}

CCmdLine::~CCmdLine()
{
}

const ICmdLineArg* CCmdLine::GetArg(int n) const
{
    if ((n >= 0) && (n < (int)m_args.size()))
    {
        return &m_args[n];
    }

    return 0;
}

int CCmdLine::GetArgCount() const
{
    return (int)m_args.size();
}

const ICmdLineArg* CCmdLine::FindArg(const ECmdLineArgType ArgType, const char* name, bool caseSensitive) const
{
    if (caseSensitive)
    {
        for (std::vector<CCmdLineArg>::const_iterator it = m_args.begin(); it != m_args.end(); ++it)
        {
            if (it->GetType() == ArgType)
            {
                if (!strcmp(it->GetName(), name))
                {
                    return &(*it);
                }
            }
        }
    }
    else
    {
        for (std::vector<CCmdLineArg>::const_iterator it = m_args.begin(); it != m_args.end(); ++it)
        {
            if (it->GetType() == ArgType)
            {
                if (!_stricmp(it->GetName(), name))
                {
                    return &(*it);
                }
            }
        }
    }

    return 0;
}


string CCmdLine::Next(char*& src)
{
    char ch = 0;
    char* org = src;

    ch = *src++;
    while (ch)
    {
        switch (ch)
        {
        case '\'':
        case '\"':
            org = src;

            while ((*src++ != ch) && *src)
            {
                ;
            }

            return string(org, src - 1);

        case '[':
            org = src;
            while ((*src++ != ']') && *src)
            {
                ;
            }
            return string(org, src - 1);

        case ' ':
            ch = *src++;
            continue;
        default:
            org = src - 1;
            for (; *src != ' ' && *src != '\t' && *src; ++src)
            {
                ;
            }

            return string(org, src);
        }
        ch = *src++;
    }

    return string();
}


