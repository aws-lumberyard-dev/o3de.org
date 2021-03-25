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

// Description : Classes to deal with commands


#ifndef CRYINCLUDE_EDITOR_INCLUDE_COMMAND_H
#define CRYINCLUDE_EDITOR_INCLUDE_COMMAND_H
#pragma once

#include <QString>

#include "Util/EditorUtils.h"

inline string ToString(const QString& s)
{
    return s.toUtf8().data();
}

class CCommand
{
public:
    CCommand(
        const string& module,
        const string& name,
        const string& description,
        const string& example)
        : m_module(module)
        , m_name(name)
        , m_description(description)
        , m_example(example)
        , m_bAlsoAvailableInScripting(false)
    {}

    virtual ~CCommand()
    {}

    // Class for storing function parameters as a type-erased string
    struct CArgs
    {
    public:
        CArgs()
            : m_stringFlags(0)
        {}

        template <typename T>
        void Add(T p)
        {
            assert(m_args.size() < 8 * sizeof(m_stringFlags));
            m_args.push_back(ToString(p));
        }
        void Add(const char* p)
        {
            assert(m_args.size() < 8 * sizeof(m_stringFlags));
            m_stringFlags |= 1 << m_args.size();
            m_args.push_back(p);
        }
        bool IsStringArg(int i) const
        {
            if (i < 0 || i >= GetArgCount())
            {
                return false;
            }

            if (m_stringFlags & (1 << i))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        int GetArgCount() const
        { return m_args.size(); }
        const string& GetArg(int i) const
        {
            assert(0 <= i && i < GetArgCount());
            return m_args[i];
        }
    private:
        DynArray<string> m_args;
        unsigned char m_stringFlags;    // This is needed to quote string parameters when logging a command.
    };

    const string& GetName() const { return m_name; }
    const string& GetModule() const { return m_module; }
    const string& GetDescription() const { return m_description; }
    const string& GetExample() const { return m_example; }

    void SetAvailableInScripting() { m_bAlsoAvailableInScripting = true; };
    bool IsAvailableInScripting() const { return m_bAlsoAvailableInScripting; }

    virtual QString Execute(const CArgs& args) = 0;

    // Only a command without any arguments and return value can be a UI command.
    virtual bool CanBeUICommand() const { return false; }

protected:
    friend class CEditorCommandManager;
    string m_module;
    string m_name;
    string m_description;
    string m_example;
    bool m_bAlsoAvailableInScripting;

    template <typename T>
    static string ToString_(T t) { return ::ToString(t); }
    static inline string ToString_(const char* val)
    { return val; }
    template <typename T>
    static bool FromString_(T& t, const char* s) { return ::FromString(t, s); }
    static inline bool FromString_(const char*& val, const char* s)
    { return (val = s) != 0; }

    void PrintHelp()
    {
        CryLogAlways("%s.%s:", m_module.c_str(), m_name.c_str());
        if (m_description.length() > 0)
        {
            CryLogAlways("    %s", m_description.c_str());
        }
        if (m_example.length() > 0)
        {
            CryLogAlways("    Usage:  %s", m_example.c_str());
        }
    }
};

class CCommand0
    : public CCommand
{
public:
    CCommand0(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<void()>& functor)
        : CCommand(module, name, description, example)
        , m_functor(functor) {}

    // UI metadata for this command, if any
    struct SUIInfo
    {
        string caption;
        string tooltip;
        string description;
        string iconFilename;
        int iconIndex;
        int commandId; // Windows command id

        SUIInfo()
            : iconIndex(0)
            , commandId(0) {}
    };

    inline QString Execute([[maybe_unused]] const CArgs& args)
    {
        assert(args.GetArgCount() == 0);

        m_functor();
        return "";
    }
    const SUIInfo& GetUIInfo() const { return m_uiInfo; }
    virtual bool CanBeUICommand() const { return true; }

protected:
    friend class CEditorCommandManager;
    AZStd::function<void()> m_functor;
    SUIInfo m_uiInfo;
};

template <typename RT>
class CCommand0wRet
    : public CCommand
{
public:
    CCommand0wRet(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<RT()>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<RT()> m_functor;
};

template <LIST(1, typename P)>
class CCommand1
    : public CCommand
{
public:
    CCommand1(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<void(LIST(1, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<void(LIST(1, P))> m_functor;
};

template <LIST(1, typename P), typename RT>
class CCommand1wRet
    : public CCommand
{
public:
    CCommand1wRet(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<RT(LIST(1, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<RT(LIST(1, P))> m_functor;
};

template <LIST(2, typename P)>
class CCommand2
    : public CCommand
{
public:
    CCommand2(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<void(LIST(2, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<void(LIST(2, P))> m_functor;
};

template <LIST(2, typename P), typename RT>
class CCommand2wRet
    : public CCommand
{
public:
    CCommand2wRet(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<RT(LIST(2, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<RT(LIST(2, P))> m_functor;
};

template <LIST(3, typename P)>
class CCommand3
    : public CCommand
{
public:
    CCommand3(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<void(LIST(3, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<void(LIST(3, P))> m_functor;
};

template <LIST(3, typename P), typename RT>
class CCommand3wRet
    : public CCommand
{
public:
    CCommand3wRet(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<RT(LIST(3, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<RT(LIST(3, P))> m_functor;
};

template <LIST(4, typename P)>
class CCommand4
    : public CCommand
{
public:
    CCommand4(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<void(LIST(4, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<void(LIST(4, P))> m_functor;
};

template <LIST(4, typename P), typename RT>
class CCommand4wRet
    : public CCommand
{
public:
    CCommand4wRet(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<RT(LIST(4, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<RT(LIST(4, P))> m_functor;
};

template <LIST(5, typename P)>
class CCommand5
    : public CCommand
{
public:
    CCommand5(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<void(LIST(5, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<void(LIST(5, P))> m_functor;
};

template <LIST(6, typename P)>
class CCommand6
    : public CCommand
{
public:
    CCommand6(const string& module, const string& name,
        const string& description, const string& example,
        const AZStd::function<void(LIST(6, P))>& functor);

    QString Execute(const CArgs& args);

protected:
    friend class CEditorCommandManager;
    AZStd::function<void(LIST(6, P))> m_functor;
};

//////////////////////////////////////////////////////////////////////////

template <typename RT>
CCommand0wRet<RT>::CCommand0wRet(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<RT()>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <typename RT>
QString CCommand0wRet<RT>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 0);

    RT ret = m_functor();
    return ToString_(ret).c_str();
}

//////////////////////////////////////////////////////////////////////////

template <LIST(1, typename P)>
CCommand1<LIST(1, P)>::CCommand1(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<void(LIST(1, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(1, typename P)>
QString CCommand1<LIST(1, P)>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 1);
    if (args.GetArgCount() < 1)
    {
        CryLogAlways("Cannot execute the command %s.%s! One argument required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    bool ok = FromString_(p1, args.GetArg(0).c_str());
    if (ok)
    {
        m_functor(p1);
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s)! Invalid argument type.",
            m_module, m_name, args.GetArg(0).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(1, typename P), typename RT>
CCommand1wRet<LIST(1, P), RT>::CCommand1wRet(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<RT(LIST(1, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(1, typename P), typename RT>
QString CCommand1wRet<LIST(1, P), RT>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 1);
    if (args.GetArgCount() < 1)
    {
        CryLogAlways("Cannot execute the command %s.%s! One argument required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    bool ok = FromString_(p1, args.GetArg(0).c_str());
    if (ok)
    {
        RT ret = m_functor(p1);
        return ToString_(ret).c_str();
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s)! Invalid argument type.",
            m_module, m_name, args.GetArg(0).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(2, typename P)>
CCommand2<LIST(2, P)>::CCommand2(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<void(LIST(2, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(2, typename P)>
QString CCommand2<LIST(2, P)>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 2);
    if (args.GetArgCount() < 2)
    {
        CryLogAlways("Cannot execute the command %s.%s! Two arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str());
    if (ok)
    {
        m_functor(p1, p2);
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s)! Invalid argument type(s).",
            m_module, m_name, args.GetArg(0).c_str(), args.GetArg(1).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(2, typename P), typename RT>
CCommand2wRet<LIST(2, P), RT>::CCommand2wRet(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<RT(LIST(2, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(2, typename P), typename RT>
QString CCommand2wRet<LIST(2, P), RT>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 2);
    if (args.GetArgCount() < 2)
    {
        CryLogAlways("Cannot execute the command %s.%s! Two arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str());
    if (ok)
    {
        RT ret = m_functor(p1, p2);
        return ToString_(ret).c_str();
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s)! Invalid argument type(s).",
            m_module, m_name, args.GetArg(0).c_str(), args.GetArg(1).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(3, typename P)>
CCommand3<LIST(3, P)>::CCommand3(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<void(LIST(3, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(3, typename P)>
QString CCommand3<LIST(3, P)>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 3);
    if (args.GetArgCount() < 3)
    {
        CryLogAlways("Cannot execute the command %s.%s! Three arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    P3 p3;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str())
        && FromString_(p3, args.GetArg(2).c_str());
    if (ok)
    {
        m_functor(p1, p2, p3);
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s,%s)! Invalid argument type(s).",
            m_module, m_name, args.GetArg(0).c_str(), args.GetArg(1).c_str(), args.GetArg(2).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(3, typename P), typename RT>
CCommand3wRet<LIST(3, P), RT>::CCommand3wRet(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<RT(LIST(3, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(3, typename P), typename RT>
QString CCommand3wRet<LIST(3, P), RT>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 3);
    if (args.GetArgCount() < 3)
    {
        CryLogAlways("Cannot execute the command %s.%s! Three arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    P3 p3;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str())
        && FromString_(p3, args.GetArg(2).c_str());
    if (ok)
    {
        RT ret = m_functor(p1, p2, p3);
        return ToString_(ret).c_str();
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s,%s)! Invalid argument type(s).",
            m_module, m_name, args.GetArg(0).c_str(), args.GetArg(1).c_str(), args.GetArg(2).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(4, typename P)>
CCommand4<LIST(4, P)>::CCommand4(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<void(LIST(4, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(4, typename P)>
QString CCommand4<LIST(4, P)>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 4);
    if (args.GetArgCount() < 4)
    {
        CryLogAlways("Cannot execute the command %s.%s! Four arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str())
        && FromString_(p3, args.GetArg(2).c_str())
        && FromString_(p4, args.GetArg(3).c_str());
    if (ok)
    {
        m_functor(p1, p2, p3, p4);
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s,%s,%s)! Invalid argument type(s).",
            m_module, m_name, args.GetArg(0).c_str(), args.GetArg(1).c_str(), args.GetArg(2).c_str(),
            args.GetArg(3).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(4, typename P), typename RT>
CCommand4wRet<LIST(4, P), RT>::CCommand4wRet(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<RT(LIST(4, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(4, typename P), typename RT>
QString CCommand4wRet<LIST(4, P), RT>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 4);
    if (args.GetArgCount() < 4)
    {
        CryLogAlways("Cannot execute the command %s.%s! Four arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str())
        && FromString_(p3, args.GetArg(2).c_str())
        && FromString_(p4, args.GetArg(3).c_str());
    if (ok)
    {
        RT ret = m_functor(p1, p2, p3, p4);
        return ToString_(ret).c_str();
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s,%s,%s)! Invalid argument type(s).",
            m_module, m_name, args.GetArg(0).c_str(), args.GetArg(1).c_str(), args.GetArg(2).c_str(),
            args.GetArg(3).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(5, typename P)>
CCommand5<LIST(5, P)>::CCommand5(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<void(LIST(5, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(5, typename P)>
QString CCommand5<LIST(5, P)>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 5);
    if (args.GetArgCount() < 5)
    {
        CryLogAlways("Cannot execute the command %s.%s! Five arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;
    P5 p5;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str())
        && FromString_(p3, args.GetArg(2).c_str())
        && FromString_(p4, args.GetArg(3).c_str())
        && FromString_(p5, args.GetArg(4).c_str());
    if (ok)
    {
        m_functor(p1, p2, p3, p4, p5);
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s,%s,%s,%s)! Invalid argument type(s).",
            m_module, m_name, args.GetArg(0).c_str(), args.GetArg(1).c_str(), args.GetArg(2).c_str(),
            args.GetArg(3).c_str(), args.GetArg(4).c_str());
        PrintHelp();
    }
    return "";
}

//////////////////////////////////////////////////////////////////////////

template <LIST(6, typename P)>
CCommand6<LIST(6, P)>::CCommand6(const string& module, const string& name,
    const string& description, const string& example,
    const AZStd::function<void(LIST(6, P))>& functor)
    : CCommand(module, name, description, example)
    , m_functor(functor)
{
}

template <LIST(6, typename P)>
QString CCommand6<LIST(6, P)>::Execute(const CCommand::CArgs& args)
{
    assert(args.GetArgCount() == 6);
    if (args.GetArgCount() < 6)
    {
        CryLogAlways("Cannot execute the command %s.%s! Six arguments required.", m_module.c_str(), m_name.c_str());
        PrintHelp();
        return "";
    }

    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;
    P5 p5;
    P6 p6;
    bool ok = FromString_(p1, args.GetArg(0).c_str())
        && FromString_(p2, args.GetArg(1).c_str())
        && FromString_(p3, args.GetArg(2).c_str())
        && FromString_(p4, args.GetArg(3).c_str())
        && FromString_(p5, args.GetArg(4).c_str())
        && FromString_(p6, args.GetArg(5).c_str());
    if (ok)
    {
        AZ_PUSH_DISABLE_WARNING(4703, "-Wunknown-warning-option")
        m_functor(p1, p2, p3, p4, p5, p6);
        AZ_POP_DISABLE_WARNING
    }
    else
    {
        CryLogAlways("Cannot execute the command %s.%s(%s,%s,%s,%s,%s,%s)! Invalid argument type(s).",
            m_module.c_str(), m_name.c_str(), args.GetArg(0).c_str(), args.GetArg(1).c_str(), args.GetArg(2).c_str(),
            args.GetArg(3).c_str(), args.GetArg(4).c_str(), args.GetArg(5).c_str());
        PrintHelp();
    }
    return "";
}
#endif // CRYINCLUDE_EDITOR_INCLUDE_COMMAND_H
