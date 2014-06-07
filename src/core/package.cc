#define	DEBUG_LEVEL_FULL

#include "core/foundation.h"
#include "core/symbol.h"
#include "core/common.h"
#include "core/corePackage.h"
#include "package.h"
#include "symbolTable.h"
#include "designators.h"
#include "symbolSet.h"
#include "hashTableEql.h"
#include "hashTableEqual.h"
#include "bignum.h"
#include "str.h"
#include "multipleValues.h"

// last include is wrappers.h
#include "wrappers.h"


namespace core
{




    
    
#define ARGS_cl_packageNicknames "(pkg)"
#define DECL_cl_packageNicknames ""
#define DOCS_cl_packageNicknames "packageNicknames"
    T_sp cl_packageNicknames(T_sp pkg)
    {_G();
	Package_sp package = coerce::packageDesignator(pkg);
	return package->getNicknames();
    };


    
    
#define ARGS_af_unintern "(symbol &optional package)"
#define DECL_af_unintern ""
#define DOCS_af_unintern "unintern"
    bool af_unintern(Symbol_sp sym, T_sp packageDesig)
    {_G();
	Package_sp pkg = coerce::packageDesignator(packageDesig);
	return pkg->unintern(sym);
    };

#define ARGS_af_findSymbol "(sym &optional package)"
#define DECL_af_findSymbol ""
#define DOCS_af_findSymbol "findSymbol"
    T_mv af_findSymbol(const string& symbolname, T_sp packageDesig)
{_G();
    Package_sp package = coerce::packageDesignator(packageDesig);
    return package->findSymbol(symbolname);
};


/*
  __BEGIN_DOC(candoScript.general.makePackage,makePackage)
  \scriptCmdRet{makePackage}{}{Text::packageName}

  Make the package.
  __END_DOC
*/

    Bignum_sp nameToKey(const char* name)
    {_G();
	return Bignum_O::create(Str_O::stringToBignum(name));
    }
    
    
#define ARGS_af_makePackage "(package-name &key nicknames use)"
#define DECL_af_makePackage ""
#define DOCS_af_makePackage "make_package"
    T_mv af_makePackage(T_sp package_name_desig, Cons_sp nick_names, T_sp use_desig)
    {_G();
	Str_sp package_name = coerce::stringDesignator(package_name_desig);
	Cons_sp use_packages = coerce::listOfPackageDesignators(use_desig);
	list<string> lnn;
	for ( Cons_sp nc = nick_names; nc.notnilp(); nc=cCdr(nc) )
	{
	    Str_sp nickstr = coerce::stringDesignator(oCar(nc));
	    lnn.push_front(nickstr->get());
	}
	list<string> lup;
	for ( Cons_sp uc = use_packages; uc.notnilp(); uc=cCdr(uc) ) {
	    lup.push_front(oCar(uc).as<Package_O>()->packageName());
	}
	return(Values(_lisp->makePackage(package_name->get(),lnn,lup)));
    }





/*
  __BEGIN_DOC(candoScript.general.listAllPackages,listAllPackages)
  \scriptCmdRet{listAllPackages}{}{Text::packageName}

  Return a list of all packages.
  __END_DOC
*/

    
    
#define ARGS_af_list_all_packages "()"
#define DECL_af_list_all_packages ""
#define DOCS_af_list_all_packages "list_all_packages"
    List_mv af_list_all_packages()
    {_G();
	Cons_sp packages = _Nil<Cons_O>();
	for ( auto mi = _lisp->packages().begin(); mi!= _lisp->packages().end(); mi++ )
	{
	    packages = Cons_O::create(*mi,packages);
	}
	return(Values(packages));
    }




/*
  __BEGIN_DOC(candoScript.general.usePackage,usePackage)
  \scriptCmdRet{usePackage}{}{Symbol::packageName}

  Use the package. Return true if we used it.
  __END_DOC
*/

    
    
#define ARGS_af_use_package "(packages-to-use-desig &optional (package-desig *package*))"
#define DECL_af_use_package ""
#define DOCS_af_use_package "SeeCLHS use-package"
    T_mv af_use_package(T_sp packages_to_use_desig, T_sp package_desig)
    {_G();
	Cons_sp packages_to_use = coerce::listOfPackageDesignators(packages_to_use_desig);
	Package_sp package = coerce::packageDesignator(package_desig);
	for ( Cons_sp cur = packages_to_use; cur.notnilp(); cur=cCdr(cur) )
	{
	    Package_sp package_to_use = oCar(cur).as<Package_O>();
	    package->usePackage(package_to_use);
	}
	return(Values(_lisp->_true()));
    }

#define ARGS_af_unuse_package "(packages-to-unuse-desig &optional (package-desig *package*))"
#define DECL_af_unuse_package ""
#define DOCS_af_unuse_package "SeeCLHS unuse-package"
    T_mv af_unuse_package(T_sp packages_to_unuse_desig, T_sp package_desig)
    {_G();
	Cons_sp packages_to_unuse = coerce::listOfPackageDesignators(packages_to_unuse_desig);
	Package_sp package = coerce::packageDesignator(package_desig);
	for ( Cons_sp cur = packages_to_unuse; cur.notnilp(); cur=cCdr(cur) )
	{
	    Package_sp package_to_unuse = oCar(cur).as<Package_O>();
	    package->unusePackage(package_to_unuse);
	}
	return(Values(_lisp->_true()));
    }


#define ARGS_af_package_shadowing_symbols "(package_desig)"
#define DECL_af_package_shadowing_symbols ""
#define DOCS_af_package_shadowing_symbols "See CLHS package_shadowing_symbols"
    T_mv af_package_shadowing_symbols(T_sp package_desig)
    {_G();
	Package_sp package = coerce::packageDesignator(package_desig);
	return(Values(package->shadowingSymbols()));
    }

/*
  __BEGIN_DOC(candoScript.general.import,import)
  \scriptCmdRet{import}{}{symbols &optional package}

  Import the symbols into the (package) or the current package.
  __END_DOC
*/
#define ARGS_af_import "(symbols-desig &optional (package-desig *package*))"
#define DECL_af_import ""
#define DOCS_af_import "See CLHS: import"
    T_mv af_import(T_sp symbols_desig, T_sp package_desig)
    {_G();
	Cons_sp symbols = coerce::listOfSymbols(symbols_desig);
	Package_sp package = coerce::packageDesignator(package_desig);
	package->import(symbols);
	return(Values(_lisp->_true()));
    }



#define ARGS_af_shadow "(symbol-names-desig &optional (package_desig *package*))"
#define DECL_af_shadow ""
#define DOCS_af_shadow "See CLHS: shadow"
    T_mv af_shadow(Cons_sp symbol_names_desig, T_sp package_desig)
    {_G();
	Cons_sp symbolNames = coerce::listOfStringDesignators(symbol_names_desig);
	Package_sp package = coerce::packageDesignator(package_desig);
	package->shadow(symbolNames);
	return(Values(_lisp->_true()));
    }

    
    
#define ARGS_af_shadowing_import "(symbol-names-desig &optional (package-desig *package*))"
#define DECL_af_shadowing_import ""
#define DOCS_af_shadowing_import "See CLHS: shadowing-import"
    T_mv af_shadowing_import(Cons_sp symbol_names_desig, T_sp package_desig)
    {_G();
	Cons_sp symbolNames = coerce::listOfSymbols(symbol_names_desig);
	Package_sp package = coerce::packageDesignator(package_desig);
	package->shadowingImport(symbolNames);
	return(Values(_lisp->_true()));
    }


















    
    static    uint	static_gentemp_counter = 1;    
#define ARGS_af_gentemp "(&optional prefix (package *package*))"
#define DECL_af_gentemp ""
#define DOCS_af_gentemp "See CLHS gentemp"
    T_mv af_gentemp(Str_sp prefix, T_sp package_designator)
    {_G();
	stringstream ss;
	string spref = "T";
	Package_sp pkg = coerce::packageDesignator(package_designator);
	if ( prefix.notnilp() ) spref = prefix->get();
	T_sp retval;
	for ( int i=0; i<1000; i++ )
	{
	    ss.str("");
	    ss << spref;
	    ss << static_gentemp_counter;
	    ++static_gentemp_counter;
	    {MULTIPLE_VALUES_CONTEXT();
		T_mv mv = pkg->findSymbol(ss.str());
		if ( mv.valueGet(1).as<T_O>().nilp() )
		{
		    {MULTIPLE_VALUES_CONTEXT();
			retval = pkg->intern(ss.str());
			goto DONE;
		    }
		}
	    }
	}
	SIMPLE_ERROR(BF("Could not find unique gentemp"));
    DONE:
	return(Values(retval));
    };


#define DOCS_af_package_use_list "package_use_list"
#define LOCK_af_package_use_list 0
#define ARGS_af_package_use_list "(package-designator)"
#define DECL_af_package_use_list ""    
    T_mv af_package_use_list(T_sp package_designator)
    {_G();
	Package_sp pkg = coerce::packageDesignator(package_designator);
	return(Values(pkg->packageUseList()));
    };


    void Package_O::exposeCando(Lisp_sp lisp)
    {_G();
	class_<Package_O>()
//	    .def("allSymbols",&Package_O::allSymbols)
	    .def("packageName",&Package_O::packageName)
	    .def("PackageHashTables",&Package_O::hashTables)
	    ;
	SYMBOL_EXPORT_SC_(ClPkg,package_use_list);
	Defun(package_use_list);
	SYMBOL_EXPORT_SC_(ClPkg,gentemp);
	Defun(gentemp);
	SYMBOL_EXPORT_SC_(ClPkg,makePackage);
	Defun(makePackage);
	SYMBOL_EXPORT_SC_(ClPkg,list_all_packages);
	Defun(list_all_packages);
	SYMBOL_EXPORT_SC_(ClPkg,use_package);
	Defun(use_package);
	SYMBOL_EXPORT_SC_(ClPkg,unuse_package);
	Defun(unuse_package);
	SYMBOL_EXPORT_SC_(ClPkg,package_shadowing_symbols);
	Defun(package_shadowing_symbols);
	SYMBOL_EXPORT_SC_(ClPkg,import);
	Defun(import);
	SYMBOL_EXPORT_SC_(ClPkg,shadow);
	Defun(shadow);
	SYMBOL_EXPORT_SC_(ClPkg,shadowing_import);
	Defun(shadowing_import);
	SYMBOL_EXPORT_SC_(ClPkg,findSymbol);
	Defun(findSymbol);
	SYMBOL_EXPORT_SC_(ClPkg,unintern);
	Defun(unintern);
	ClDefun(packageNicknames);
    }

    void Package_O::exposePython(Lisp_sp lisp)
    {_G();
#ifdef	USEBOOSTPYTHON //[
	PYTHON_CLASS(CorePkg,Package,"","",_lisp)
//	    .def("allSymbols",&Package_O::allSymbols)
	    ;
#endif //]
    }


    Package_sp Package_O::create( const string& name)
    {
	Package_sp p = Package_O::create();
	p->setName(name);
	return p;
    }



    void	Package_O::initialize()
    {
	this->Base::initialize();
	this->_InternalSymbols = HashTableEql_O::create_default();
	this->_ExternalSymbols = HashTableEql_O::create_default();
        this->_ShadowingSymbols = HashTableEqual_O::create_default();
	this->_KeywordPackage = false;
	this->_AmpPackage = false;
    }


    T_mv Package_O::hashTables() const
    {
	Cons_sp useList = _Nil<Cons_O>();
	for ( auto ci=this->_UsingPackages.begin();
	      ci!=this->_UsingPackages.end(); ci++ )
	{
	    useList = Cons_O::create(*ci,useList);
	}
	return(Values(this->_ExternalSymbols, this->_InternalSymbols, useList));
    }

    string	Package_O::__repr__() const
    {
	stringstream ss;
	ss << "#<" << this->_Name << ">";
	return ss.str();
    }

#if defined(XML_ARCHIVE)
    void	Package_O::archiveBase(ArchiveP node)
    {
	IMPLEMENT_MEF(BF("Handle archiving the package hash-tables"));
#if 0
	this->Base::archiveBase(node);
	node->attribute("name",this->_Name);
	node->archiveMap("internalSymbols",this->_InternalSymbols);
	node->archiveMap("externalSymbols",this->_ExternalSymbols);
	node->archiveSetIfDefined("usingPackages",this->_UsingPackages);
	node->archiveMapIfDefined("shadowingSymbols",this->_ShadowingSymbols);
#endif
    }
#endif // defined(XML_ARCHIVE)

    class PackageMapper : public KeyValueMapper
    {
    public:
	stringstream* ssP;
	string type;
	PackageMapper(const string& t, stringstream* sstr) : ssP(sstr), type(t) {};
	virtual bool mapKeyValue(T_sp key, T_sp value)
	{
	    (*(this->ssP)) << type << " " << _rep_(value) << std::endl;
	    return true;
	}
    };


    string Package_O::allSymbols()
    {_G();
	stringstream ss;
	PackageMapper internals("internal",&ss);
	this->_InternalSymbols->lowLevelMapHash(&internals);
	PackageMapper externals("external",&ss);
	this->_ExternalSymbols->lowLevelMapHash(&externals);
	return ss.str();
    }


    Symbol_mv Package_O::findSymbolDirectlyContained(Bignum_sp nameKey) const
    {_G();
	// Look in ShadowingSymbols
	{
	    Symbol_sp val;
	    bool foundp;
	    {MULTIPLE_VALUES_CONTEXT();
		T_mv ei = this->_ExternalSymbols->gethash(nameKey,_Nil<T_O>());
		val = ei.as<Symbol_O>();
		foundp = ei.valueGet(1).as<T_O>().isTrue();
	    }
	    if ( foundp )
	    {
		LOG(BF("Found it in the _ExternalsSymbols list - returning[%s]") % (_rep_(val) ));
		return(Values(val,kw::_sym_external));
	    }
	}
	{
	    bool foundp;
	    Symbol_sp first;
	    {MULTIPLE_VALUES_CONTEXT();
		T_mv ej = this->_InternalSymbols->gethash(nameKey,_Nil<T_O>());
		first = ej.as<Symbol_O>();
		foundp = ej.valueGet(1).as<T_O>().isTrue();
	    }
	    if ( foundp )
	    {
		LOG(BF("Found it in the _InternalSymbols list - returning[%s]") % (_rep_(first) ));
		return(Values(first,kw::_sym_internal));
	    }
	}
	return( Values0<Symbol_O>() );
    }



    Symbol_mv Package_O::findSymbol(Bignum_sp nameKey) const
    {_G();
	Symbol_sp retval;
	Symbol_sp retstatus;
	T_mv mv = this->findSymbolDirectlyContained(nameKey);
	int isize = mv.number_of_values();
	if ( isize != 0 ) {
	    retval = mv.as<Symbol_O>();
	    retstatus = mv.valueGet(1).as<Symbol_O>();
	    return(Values(retval,retstatus));
	}
	{_BLOCK_TRACEF(BF("Looking in _UsingPackages"));
	    for ( auto it = this->_UsingPackages.begin();
		  it != this->_UsingPackages.end(); it++ )
	    {
		Package_sp pkg = *it;
		LOG(BF("Looking in package[%s]") % _rep_(pkg) );
		T_mv tmv = pkg->findSymbolDirectlyContained(nameKey);
		if ( tmv.number_of_values()==0 ) continue;
		Symbol_sp uf = tmv.as<Symbol_O>();
		Symbol_sp status = tmv.valueGet(1).as<Symbol_O>();
		if ( status.notnilp() )
		{
		    if (status != kw::_sym_external) continue;
		    LOG(BF("Found exported symbol[%s]") % _rep_(uf) );
		    return(Values(uf,kw::_sym_inherited));
		}
	    }
	}
	return(Values(_Nil<Symbol_O>(),_Nil<Symbol_O>()));
    }



    Symbol_mv Package_O::findSymbol(const string& name) const
    {_G();
	Bignum_sp nameKey = nameToKey(name.c_str());
	return findSymbol(nameKey);
    }


    

    Cons_sp Package_O::packageUseList()
    {_OF();
	Cons_sp res = _Nil<Cons_O>();
	for ( auto si=this->_UsingPackages.begin();
	      si!=this->_UsingPackages.end(); si++ )
	{
	    res = Cons_O::create(*si,res);
	}
	return res;
    }





    T_mv Package_O::packageHashTables() const
    {_G();
	Cons_sp usingPackages = _Nil<Cons_O>();
	for ( auto si=this->_UsingPackages.begin();
	      si!=this->_UsingPackages.end(); si++ )
	{
	    usingPackages = Cons_O::create(*si,usingPackages);
	}
	return(Values(this->_ExternalSymbols,
		       this->_InternalSymbols,
		       usingPackages));
    }

    bool Package_O::usingPackageP(Package_sp usePackage) const
    {
	for ( auto it= this->_UsingPackages.begin(); it!=this->_UsingPackages.end(); ++it )
	{
	    if ( (*it) == usePackage ) return true;
	}
	return false;
    }



     bool Package_O::usePackage(Package_sp usePackage)
     {_OF();
	LOG(BF("In usePackage this[%s]  using package[%s]") % this->getName() % usePackage->getName() );
	if ( this->usingPackageP(usePackage) )
 	{
	    LOG(BF("You are already using that package"));
	    return true;
	}
	// 
	// Check for symbol conflicts
	//
	FindConflicts findConflicts(this->asSmartPtr());
	this->_ExternalSymbols->lowLevelMapHash(&findConflicts);
	if ( findConflicts._conflicts.size() > 0 )
	{
	    stringstream ss;
	    for ( set<string>::iterator si=findConflicts._conflicts.begin();
		  si!=findConflicts._conflicts.end(); si++ )
	    {
		ss << " " << *si;
 	    }
	    SIMPLE_ERROR(BF("Error: Name conflict when importing package[%s]"
				  " into package[%s] - conflicting symbols: %s")
			       % usePackage->getName()
			       % this->getName()
			       % (ss.str()));
 	}
	this->_UsingPackages.push_back(usePackage);
 	return true;
     }
 


    bool Package_O::unusePackage(Package_sp usePackage)
    {_OF();
	for ( auto it= this->_UsingPackages.begin();
	      it!=this->_UsingPackages.end(); ++it )
	{
	    if ( (*it) == usePackage ) {
		this->_UsingPackages.erase(it);
		return true;
	    }
	}
	return true;
    }



    void Package_O::_export(Cons_sp symbols)
    {_OF();
	for ( Cons_sp cur = symbols; cur.notnilp(); cur = cCdr(cur) )
	{
	    Symbol_sp sym = oCar(cur).as<Symbol_O>();
	    if ( sym.nilp() ) continue;
	    if ( sym->symbolNameAsString() == "" )
	    {
		SIMPLE_ERROR(BF("Problem exporting symbol - it has no name"));
	    }
	    Bignum_sp nameKey = nameToKey(sym->_Name->c_str());
	    Symbol_sp foundSym, status;
	    {MULTIPLE_VALUES_CONTEXT();
		T_mv values = this->findSymbol(nameKey);
		foundSym = values.as<Symbol_O>();
		status = values.valueGet(1).as<Symbol_O>();
	    }
	    LOG(BF("findSymbol status[%s]") % _rep_(status) );
	    if ( status.nilp() )
	    {
		this->_ExternalSymbols->hash_table_setf_gethash(nameKey,sym);
	    } else if ( status == kw::_sym_external )
	    {
		LOG(BF("Symbol[%s] is already in _ExternalSymbols - nothing to do") % _rep_(sym) );
		// do nothing its already external
	    } else if ( status == kw::_sym_internal )
	    {
		LOG(BF("Moving symbol[%s] into _ExternalSymbols") % _rep_(sym) );
		this->_InternalSymbols->remhash(nameKey);
		this->_ExternalSymbols->hash_table_setf_gethash(nameKey,sym);
	    } else if ( status == kw::_sym_inherited )
	    {
		LOG(BF("Symbol[%s] was inherited - importing it and then exporting it") % _rep_(sym) );
		this->import(Cons_O::create(sym));
		this->_export(Cons_O::create(sym));
	    }
	}
    }


    bool Package_O::shadow(Str_sp symbolName)
    {_G();
        Symbol_sp shadowSym, status;
        Symbol_mv values = this->findSymbol(symbolName->get());
        shadowSym = values;
        status = values.valueGet(1).as<Symbol_O>();
        if ( status.nilp() || (status != kw::_sym_internal && status!=kw::_sym_external) )
        {
            shadowSym = Symbol_O::create(symbolName->get());
            shadowSym->makunbound();
            shadowSym->setPackage(this->sharedThis<Package_O>());
            LOG(BF("Created symbol<%s>") % _rep_(shadowSym) );
            this->_add_symbol_to_package(shadowSym);
        }
        this->_ShadowingSymbols->setf_gethash(symbolName,shadowSym);
	return true;
    }



    bool Package_O::shadow(Cons_sp symbolNames)
    {_OF();
	for ( Cons_sp cur = symbolNames; cur.notnilp(); cur = cCdr(cur) )
	{
	    Str_sp name = oCar(cur).as<Str_O>();
            this->shadow(name);
        }
	return true;
    }


    void trapSymbol(Package_O* pkg, Symbol_sp sym, const string& name)
    {
#if 0
        // Dump every symbol
        printf("INTERNING symbol[%s]@%p\n", name.c_str(), sym.px_ref() );
#endif
#if 0
	if ( name[0] == '+') // == "+HASH-TABLE-ENTRY-VALUE-HAS-BEEN-DELETED+")
	{
	    printf("%s:%d - Package_O::intern of %s@%p in package %s\n",
		   __FILE__, __LINE__, name.c_str(), sym.px_ref(), pkg->getName().c_str() );
	}
#endif
    }



    void Package_O::_add_symbol_to_package(Symbol_sp sym, bool exportp)
    {
//	trapSymbol(this,sym,sym->_Name->get());
	Bignum_sp nameKey = nameToKey(sym->_Name->c_str());
	if ( this->isKeywordPackage() || exportp )
	{
	    this->_ExternalSymbols->hash_table_setf_gethash(nameKey,sym);
	} else
	{
	    this->_InternalSymbols->hash_table_setf_gethash(nameKey,sym);
	}
	if ( llvm_interface::addSymbol != NULL )
	{
	    llvm_interface::addSymbol(sym);
	}
    }


    T_mv Package_O::intern(const string& name)
    {_OF();
	Symbol_mv values = this->findSymbol(name);
	Symbol_sp sym = values;
	Symbol_sp status = values.valueGet(1).as<Symbol_O>();
	if ( status.nilp() )
	{
	    sym = Symbol_O::create(name);
	    sym->makunbound();
	    status = _Nil<Symbol_O>();
	    sym->setPackage(this->sharedThis<Package_O>());
	    LOG(BF("Created symbol<%s>") % _rep_(sym) );
	    this->_add_symbol_to_package(sym);
	}
	if ( this->isKeywordPackage() )
	{
	    sym->setf_symbolValue(sym);
	}
//	trapSymbol(this,sym,name);
	LOG(BF("Symbol[%s] interned as[%s]@%p")% name % _rep_(sym) % sym.get() );
	return(Values(sym,status) );
    }



    bool Package_O::unintern(Symbol_sp sym)
    {_OF();
	LOG(BF("About to unintern symbol[%s]") % _rep_(sym) );
	// The following is not completely conformant with CLHS
	// unintern should throw an exception if removing a shadowing symbol
	// uncovers a name conflict of the symbol in two packages that are being used
	Bignum_sp nameKey = nameToKey(sym->_Name->c_str());

	{
	    Symbol_sp sym, status;
	    {MULTIPLE_VALUES_CONTEXT();
		Symbol_mv values = this->findSymbol(nameKey);
		sym = values;
		status = values.valueGet(1).as<Symbol_O>();
	    }
	    if ( status.notnilp() )
	    {
		if ( this->_ShadowingSymbols->contains(sym->symbolName()) )
		{
		    this->_ShadowingSymbols->remhash(sym->symbolName());
		}
		if ( status == kw::_sym_internal )
		{
		    this->_InternalSymbols->remhash(nameKey);
		    if (sym->getPackage().get() == this) sym->setPackage(_Nil<Package_O>());
		    return true;
		} else if ( status == kw::_sym_external)
		{
		    this->_ExternalSymbols->remhash(nameKey);
		    if (sym->getPackage().get() == this) sym->setPackage(_Nil<Package_O>());
		    return true;
		}
	    }
	}
	{_BLOCK_TRACEF(BF("Looking in _UsingPackages"));
	    SymbolSet_sp used_symbols(SymbolSet_O::create());
	    for ( auto it = this->_UsingPackages.begin();
		  it != this->_UsingPackages.end(); it++ )
	    {
		Symbol_sp uf, status;
		{MULTIPLE_VALUES_CONTEXT();
		    Symbol_mv values = (*it)->findSymbol(nameKey);
		    uf = values;
		    status = values.valueGet(1).as<Symbol_O>();
		}
		if ( status.notnilp() )
		{
		    if (status != kw::_sym_external) continue;
		    used_symbols->insert(uf);
		}
	    }
	    if ( used_symbols->size() > 0 )
	    {
		SIMPLE_ERROR(BF("unintern symbol[%s] revealed name collision with used packages containing symbols: %s") % _rep_(sym) % _rep_(used_symbols->asCons()));
	    }
	}
	return false;
    }


    bool Package_O::isExported(Symbol_sp sym)
    {_OF();
	Bignum_sp nameKey = nameToKey(sym->_Name->c_str());
	T_mv values = this->_ExternalSymbols->gethash(nameKey,_Nil<T_O>());
	T_sp presentp = values.valueGet(1);
    	LOG(BF("isExported test of symbol[%s] isExported[%d]") % sym->symbolNameAsString() % presentp.isTrue() );
	return(presentp.isTrue());
    }




    void Package_O::import(Cons_sp symbols)
    {_OF();
	for ( Cons_sp cur = symbols; cur.notnilp(); cur = cCdr(cur) )
	{
	    Symbol_sp symbolToImport = oCar(cur).as<Symbol_O>();
	    Bignum_sp nameKey = nameToKey(symbolToImport->_Name->c_str());
	    Symbol_sp foundSymbol, status;
	    {MULTIPLE_VALUES_CONTEXT();
		Symbol_mv values = this->findSymbol(nameKey);
		foundSymbol = values;
		status = values.valueGet(1).as<Symbol_O>();
	    }
	    if ( status == kw::_sym_external || status == kw::_sym_internal )
	    {
		// do nothing
	    } else if ( status == kw::_sym_inherited || status.nilp() )
	    {
		this->_InternalSymbols->hash_table_setf_gethash(nameKey,symbolToImport);
	    } else
	    {
		PACKAGE_ERROR(this->sharedThis<Package_O>());
	    }
	}
    }


    void Package_O::shadowingImport(Cons_sp symbols)
    {_OF();
	for ( Cons_sp cur = symbols; cur.notnilp(); cur = cCdr(cur) )
	{
	    Symbol_sp symbolToImport = oCar(cur).as<Symbol_O>();
	    Bignum_sp nameKey = nameToKey(symbolToImport->_Name->c_str());
	    Symbol_sp foundSymbol, status;
	    {MULTIPLE_VALUES_CONTEXT();
		Symbol_mv values = this->findSymbol(nameKey);
		foundSymbol = values;
		status = values.valueGet(1).as<Symbol_O>();
	    }
	    if ( foundSymbol != symbolToImport )
	    {
		if ( status != kw::_sym_inherited )
		{
		    this->unintern(foundSymbol);
		}
		this->_InternalSymbols->hash_table_setf_gethash(nameKey,symbolToImport);
	    }
	    this->_ShadowingSymbols->setf_gethash(symbolToImport->symbolName(),symbolToImport);
	}
    }


    Cons_sp Package_O::shadowingSymbols() const
    {_OF();
	Cons_sp cur = _Nil<Cons_O>();
        this->_ShadowingSymbols->mapHash( [&cur] (T_sp name, T_sp symbol) {
	    cur = Cons_O::create(symbol,cur);
            } );
	return cur;
    }



void Package_O::mapExternals(KeyValueMapper* mapper)
{_G();
    this->_ExternalSymbols->lowLevelMapHash(mapper);
}

void Package_O::mapInternals(KeyValueMapper* mapper)
{_G();
    this->_InternalSymbols->lowLevelMapHash(mapper);
}









    void Package_O::dumpSymbols()
    {_OF();
	string all = this->allSymbols();
	_lisp->print(BF("%s")%all);
    }

    EXPOSE_CLASS(core,Package_O);

};




