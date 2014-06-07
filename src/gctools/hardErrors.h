
#ifndef gc_hardErrors_H
#define gc_hardErrors_H


void dbg_hook(const char* errorString);

namespace core {
    void errorFormatted(boost::format fmt);
    void errorFormatted(const char* errorString);
    void errorFormatted(const string& msg);


    class HardError : public gctools::GCIgnoreClass
    {
    private:
        string _Message;
    public:
        HardError(const char* sourceFile, const char* functionName, uint lineNumber, const boost::format& fmt);
        HardError(const char* sourceFile, const char* functionName, uint lineNumber, const string& msg);
        string message();
    };

};



#define THROW_HARD_ERROR(fmt) {dbg_hook((fmt).str().c_str());core::errorFormatted(fmt);throw(core::HardError(__FILE__,__FUNCTION__,__LINE__,fmt));}
#define HARD_SUBCLASS_MUST_IMPLEMENT() THROW_HARD_ERROR(boost::format("Subclass must implement"));
#ifdef DEBUG_ASSERTS
#define GCTOOLS_ASSERT(x) {if (!(x)) THROW_HARD_ERROR(BF("Failed gctools assertion %s") % #x);};
#define GCTOOLS_ASSERTF(x,fmt) {if (!(x)) THROW_HARD_ERROR(fmt);};
#else
#define GCTOOLS_ASSERT(x)
#define GCTOOLS_ASSERTF(x,f)
#endif

#endif // gc_hardErrors_H
