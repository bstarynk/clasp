(in-package :cscrape)

(defparameter +root-dummy-class+ "::_RootDummyClass")
(define-condition bad-c++-name (error)
  ((name :initarg :name :accessor name))
  (:report (lambda (condition stream)
             (format stream "Bad C++ function name: ~a" (name condition)))))

(defun group-expose-functions-by-namespace (functions)
  (declare (optimize (speed 3)))
  (let ((ns-hashes (make-hash-table :test #'equal)))
    (dolist (func functions)
      (let* ((namespace (namespace% func))
             (ns-ht (gethash namespace ns-hashes (make-hash-table :test #'equal))))
        (setf (gethash (lisp-name% func) ns-ht) func)
        (setf (gethash namespace ns-hashes) ns-ht)))
    ns-hashes))

(defun generate-expose-function-signatures (sout ns-grouped-expose-functions)
  (format sout "#ifdef EXPOSE_FUNCTION_SIGNATURES~%")
  (maphash (lambda (ns func-ht)
             (format sout "namespace ~a {~%" ns)
             (maphash (lambda (name f)
                        (declare (ignore name))
                        (when (and (typep f 'expose-defun)
                                   (provide-declaration% f))
                          (format sout "    ~a;~%" (signature% f))))
                      func-ht)
             (format sout "};~%"))
           ns-grouped-expose-functions)
  (format sout "#endif // EXPOSE_FUNCTION_SIGNATURES~%"))

#+(or)(defun split-c++-name (name)
        (declare (optimize (speed 3)))
        (let ((under (search "__" name :test #'string=)))
          (unless under
            (error 'bad-c++-name :name name))
          (let* ((name-pos (+ 2 under)))
            (values (subseq name 0 under)
                    (subseq name name-pos)))))

(defun maybe-wrap-lambda-list (ll)
  (if (> (length ll) 0)
      (format nil "(~a)" ll)
      ll))
(defun generate-expose-function-bindings (sout ns-grouped-expose-functions)
  (declare (optimize (speed 3)))
  (flet ((expose-one (f ns index)
           (let ((name (format nil "expose_function_~d_helper" index)))
             (format sout "NOINLINE void ~a() {~%" name)
             (etypecase f
               (expose-defun
                (format sout "  expose_function(~a,~a,&~a::~a,~s);~%"
                        (lisp-name% f)
                        "true"
                        ns
                        (function-name% f)
                        (maybe-wrap-lambda-list (lambda-list% f))))
               (expose-extern-defun
                (format sout "  expose_function(~a,~a,~a,~s);~%"
                        (lisp-name% f)
                        "true"
                        (pointer% f)
                        (maybe-wrap-lambda-list (lambda-list% f)))))
             (format sout "}~%")
             name)))
    (let (helpers (index 0))
      (format sout "#ifdef EXPOSE_FUNCTION_BINDINGS_HELPERS~%")
      (maphash (lambda (ns funcs-ht)
                 (maphash (lambda (name f)
                            (declare (ignore name))
                            (handler-case
                                (push (expose-one f ns (incf index)) helpers)
                              (serious-condition (condition)
                                (error "There was an error while exposing a function in ~a at line ~d~%~a~%" (file% f) (line% f) condition))))
                          funcs-ht))
               ns-grouped-expose-functions)
      (format sout "#endif // EXPOSE_FUNCTION_BINDINGS_HELPERS~%")
      (format sout "#ifdef EXPOSE_FUNCTION_BINDINGS~%")
      (dolist (helper (nreverse helpers))
        (format sout "  ~a();~%" helper))
      (format sout "#endif // EXPOSE_FUNCTION_BINDINGS~%"))))

(defun generate-expose-one-source-info-helper (sout obj idx)
  (let* ((lisp-name (lisp-name% obj))
         (absolute-file (truename (pathname (file% obj))))
         (file (enough-namestring absolute-file (pathname (format nil "~a/" (sb-ext:posix-getenv "CLASP_HOME")))))
         (line (line% obj))
         (char-offset (character-offset% obj))
         (docstring (docstring% obj))
         (kind (cond
                 ((typep obj 'function-mixin) "code_kind")
                 ((typep obj 'class-method-mixin) "code_kind")
                 ((typep obj 'method-mixin) "method_kind")
                 ((typep obj 'exposed-class) "class_kind")
                 (t "unknown_kind")))
         (helper-name (format nil "source_info_~d_helper" idx)))
    (format sout "NOINLINE void source_info_~d_helper() {~%" idx)
    (format sout " define_source_info( ~a, ~a, ~s, ~d, ~d, ~a );~%"
            kind lisp-name file char-offset line docstring )
    (format sout "}~%")
    helper-name))

(defun generate-expose-source-info (sout functions classes)
  (declare (optimize debug))
  (let (helpers
        (index 0))
    (format sout "#ifdef SOURCE_INFO_HELPERS~%")
    (dolist (f functions)
      (push (generate-expose-one-source-info-helper sout f (incf index)) helpers))
    (maphash (lambda (k class)
               (push (generate-expose-one-source-info-helper sout class (incf index)) helpers)
               (dolist (method (methods% class))
                 (push (generate-expose-one-source-info-helper sout method (incf index)) helpers))
               (dolist (class-method (class-methods% class))
                 (push (generate-expose-one-source-info-helper sout class-method (incf index)) helpers)))
           classes)
    (format sout "#endif // SOURCE_INFO_HELPERS~%")
    (format sout "#ifdef SOURCE_INFO~%")
    (dolist (helper (nreverse helpers))
      (format sout "  ~a();~%" helper))
    (format sout "#endif // SOURCE_INFO~%")))

(defun generate-code-for-source-info (functions classes)
  (with-output-to-string (sout)
    (generate-expose-source-info sout functions classes)))


#+(or)(defun generate-tags-file (tags-file-name tags)
        (declare (optimize (speed 3)))
        (let* ((source-info-tags (extract-unique-source-info-tags tags))
               (file-ht (make-hash-table :test #'equal)))
          (dolist (tag source-info-tags)
            (push tag (gethash (tags:file tag) file-ht)))
          (let ((tags-data-ht (make-hash-table :test #'equal)))
            (maphash (lambda (file-name file-tags-list)
                       (let ((buffer (make-string-output-stream #+(or):element-type #+(or)'(unsigned-byte 8))))
                         (dolist (tag file-tags-list)
                           (format buffer "~a~a~a,~a~%"
                                   (tags:function-name tag)
                                   (code-char #x7f)
                                   (tags:line tag)
                                   (tags:character-offset tag)))
                         (setf (gethash file-name tags-data-ht) (get-output-stream-string buffer))))
                     file-ht)
            (with-open-file (sout tags-file-name :direction :output #+(or):element-type #+(or)'(unsigned-byte 8)
                                  :if-exists :supersede)
              (maphash (lambda (file buffer)
                         (format sout "~a,~a~%"
                                 file
                                 (length buffer))
                         (princ buffer sout))
                       tags-data-ht)))))

(defun generate-code-for-init-functions (functions)
  (declare (optimize (speed 3)))
  (with-output-to-string (sout)
    (let ((ns-grouped (group-expose-functions-by-namespace functions)))
      (generate-expose-function-signatures sout ns-grouped)
      (generate-expose-function-bindings sout ns-grouped))))

(defun mangle-and-wrap-name (name)
  "* Arguments
- name :: A string
* Description
Convert colons to underscores"
  (format nil "wrapped_~a" (substitute #\_ #\: name)))

(defgeneric direct-call-function (c-code cl-code func c-code-info cl-code-info))

(defmethod direct-call-function (c-code cl-code (func t) c-code-info cl-code-info)
  (format c-code "// Do nothing yet for function ~a of type ~a~%" (function-name% func) (type-of func))
  (format cl-code ";;; Do nothing yet for function ~a of type ~a~%" (function-name% func) (type-of func)))

(defmethod direct-call-function (c-code cl-code (func expose-defun) c-code-info cl-code-info)
  (multiple-value-bind (return-type arg-types)
      (parse-types-from-signature (signature% func))
    (let* ((wrapped-name (mangle-and-wrap-name (function-name% func)))
           (one-func-code
            (generate-wrapped-function wrapped-name
                                       (namespace% func)
                                       (function-name% func)
                                       return-type arg-types)))
      (format c-code "// Generating code for ~a::~a~%" (namespace% func) (function-name% func))
      (format c-code-info "// Generating code for ~a::~a~%" (namespace% func) (function-name% func))
      (format c-code-info "//            Found at ~a:~a~%-----------~%" (file% func) (line% func))
      (format c-code "~a~%" one-func-code)
      (format cl-code ";;; Generating code for ~a::~a~%" (namespace% func) (function-name% func))
      (format cl-code-info ";;; Generating code for ~a::~a~%" (namespace% func) (function-name% func))
      (format cl-code-info ";;;            Found at ~a:~a~%----------~%" (file% func) (line% func))
      (let* ((raw-lisp-name (lisp-name% func))
             (maybe-fixed-magic-name (maybe-fix-magic-name raw-lisp-name)))
        (format cl-code "(generate-direct-call-defun ~a (~a) ~s )~%" maybe-fixed-magic-name (lambda-list% func) wrapped-name )))))
                               
(defun generate-code-for-direct-call-functions (functions)
  (let ((c-code (make-string-output-stream))
        (c-code-info (make-string-output-stream))
        (cl-code (make-string-output-stream))
        (cl-code-info (make-string-output-stream)))
    (format cl-code "(in-package :core)~%")
    (mapc (lambda (func)
            (direct-call-function c-code cl-code func c-code-info cl-code-info))
          functions)
    (values (get-output-stream-string c-code)
            (get-output-stream-string cl-code)
            (get-output-stream-string c-code-info)
            (get-output-stream-string cl-code-info))))

(define-condition broken-inheritance (error)
    ((class-with-missing-parent :initarg :class-with-missing-parent :accessor class-with-missing-parent)
     (starting-possible-child-class :initarg :starting-possible-child-class :accessor starting-possible-child-class)
     (possible-ancestor-class :initarg :possible-ancestor-class :accessor possible-ancestor-class))
  (:report (lambda (condition stream)
             (format stream "While testing if the ancestor of ~a is ~a ~%   the chain of inheritance was broken when no parent could be found for ~a~%"
                     (starting-possible-child-class condition)
                     (possible-ancestor-class condition)
                     (class-with-missing-parent condition)))))

(define-condition possible-recursive-inheritance (error)
    ((class-with-missing-parent :initarg :class-with-missing-parent :accessor class-with-missing-parent)
     (starting-possible-child-class :initarg :starting-possible-child-class :accessor starting-possible-child-class)
     (possible-ancestor-class :initarg :possible-ancestor-class :accessor possible-ancestor-class))
  (:report (lambda (condition stream)
             (format stream "While testing if the ancestor of ~a is ~a ~%   the chain of inheritance was broken when no parent could be found for ~a~%"
                     (starting-possible-child-class condition)
                     (possible-ancestor-class condition)
                     (class-with-missing-parent condition)))))

      
(defun inherits-from* (x-name y-name inheritance)
  (let ((depth 0)
        ancestor
        (entry-x-name x-name)
        prev-ancestor)
    (loop
       (setf prev-ancestor ancestor
             ancestor (gethash x-name inheritance))
       (when (string= ancestor +root-dummy-class+)
         (return-from inherits-from* nil))
       (unless ancestor
         (error 'broken-inheritance
                :class-with-missing-parent x-name
                :starting-possible-child-class entry-x-name
                :possible-ancestor-class y-name))
       (if (string= ancestor y-name)
           (return-from inherits-from* t))
       (incf depth)
       (when (> depth 20)
         (error "While testing if ~a inherits from ~a the maximum depth test was hit - current class is ~a - check for recursive class definition of ~a" entry-x-name y-name x-name entry-x-name))
       (setf x-name ancestor))))

(defun inherits-from (x y inheritance)
  (declare (optimize debug))
  (let ((x-name (class-key% x))
        (y-name (class-key% y)))
    (inherits-from* x-name y-name inheritance)))

(defparameter *classes* nil)
(defparameter *inheritance* nil)
(defun sort-classes-by-inheritance (exposed-classes)
  (declare (optimize debug))
  (let ((inheritance (make-hash-table :test #'equal))
        (classes nil))
    (maphash (lambda (k v)
               (let ((base (base% v)))
                 (when base (setf (gethash k inheritance) base))
                 (push v classes)))
             exposed-classes)
    (setf *classes* classes)
    (setf *inheritance* inheritance)
    (handler-case
        (sort classes (lambda (x y)
                        (not (inherits-from x y inheritance))))
      (broken-inheritance (e)
        (print-object e t)
        (let ((x (starting-possible-child-class e)))
          (error "~a~%    The info for ~a is ~a"
                 (with-output-to-string (sout) (print-object e sout))
                 x
                 (gethash x exposed-classes)))))))

(defun generate-code-for-init-classes-class-symbols (exposed-classes sout)
  (declare (optimize (speed 3)))
  (let ((sorted-classes (sort-classes-by-inheritance exposed-classes))
        cur-package)
    (format sout "#ifdef SET_CLASS_SYMBOLS~%")
    (dolist (exposed-class sorted-classes)
      (format sout "set_one_static_class_symbol<~a::~a>(bootStrapSymbolMap,~a);~%"
              (tags:namespace% (class-tag% exposed-class))
              (tags:name% (class-tag% exposed-class))
              (lisp-name% exposed-class)))
    (format sout "#endif // SET_CLASS_SYMBOLS~%")))

(defun as-var-name (ns name)
  (format nil "~a_~a_var" ns name))

(defun generate-code-for-init-classes-and-methods (exposed-classes)
  (declare (optimize (speed 3)))
  (with-output-to-string (sout)
    (let ((sorted-classes (sort-classes-by-inheritance exposed-classes))
          cur-package)
      (generate-code-for-init-classes-class-symbols exposed-classes sout)
      (progn
        (format sout "#ifdef ALLOCATE_ALL_CLASSES~%")
        (dolist (exposed-class sorted-classes)
          (format sout "gctools::smart_ptr<~a> ~a = allocate_one_class<~a::~a,~a>();~%"
                  (meta-class% exposed-class)
                  (as-var-name (tags:namespace% (class-tag% exposed-class))
                               (tags:name% (class-tag% exposed-class)))
                  (tags:namespace% (class-tag% exposed-class))
                  (tags:name% (class-tag% exposed-class))
                  (meta-class% exposed-class)))
        (format sout "#endif // ALLOCATE_ALL_CLASSES~%"))
      (progn
        (format sout "#ifdef SET_BASES_ALL_CLASSES~%")
        (dolist (exposed-class sorted-classes)
          (unless (string= (base% exposed-class) +root-dummy-class+)
            (format sout "~a->addInstanceBaseClassDoNotCalculateClassPrecedenceList(~a::static_classSymbol());~%"
                    (as-var-name (tags:namespace% (class-tag% exposed-class))
                                 (tags:name% (class-tag% exposed-class)))
                    (base% exposed-class))))
        (format sout "#endif // SET_BASES_ALL_CLASSES~%"))
      (progn
        (format sout "#ifdef CALCULATE_CLASS_PRECEDENCE_ALL_CLASSES~%")
        (dolist (exposed-class sorted-classes)
          (unless (string= (base% exposed-class) +root-dummy-class+)
            (format sout "~a->__setupStage3NameAndCalculateClassPrecedenceList(~a::~a::static_classSymbol());~%"
                    (as-var-name (tags:namespace% (class-tag% exposed-class))
                                 (tags:name% (class-tag% exposed-class)))
                    (tags:namespace% (class-tag% exposed-class))
                    (tags:name% (class-tag% exposed-class)))))
        (format sout "#endif //#ifdef CALCULATE_CLASS_PRECEDENCE_ALL_CLASSES~%"))
      (progn
        (format sout "#ifdef EXPOSE_CLASSES_AND_METHODS~%")
        (dolist (exposed-class sorted-classes)
          (format sout "~a::~a::expose_to_clasp();~%"
                  (tags:namespace% (class-tag% exposed-class))
                  (tags:name% (class-tag% exposed-class))))
        (format sout "#endif //#ifdef EXPOSE_CLASSES_AND_METHODS~%"))
      (progn
        (format sout "#ifdef EXPOSE_CLASSES~%")
        (dolist (exposed-class sorted-classes)
          (when (string/= cur-package (package% exposed-class))
            (when cur-package (format sout "#endif~%"))
            (setf cur-package (package% exposed-class))
            (format sout "#ifdef Use_~a~%" cur-package))
          (format sout "DO_CLASS(~a,~a,~a,~a,~a,~a);~%"
                  (tags:namespace% (class-tag% exposed-class))
                  (subseq (class-key% exposed-class) (+ 2 (search "::" (class-key% exposed-class))))
                  (package% exposed-class)
                  (lisp-name% exposed-class)
                  (base% exposed-class)
                  (meta-class% exposed-class)))
        (format sout "#endif~%")
        (format sout "#endif // EXPOSE_CLASSES~%"))
      (progn
        (format sout "#ifdef EXPOSE_STATIC_CLASS_VARIABLES~%")
        (dolist (exposed-class sorted-classes)
          (let ((class-tag (class-tag% exposed-class)))
            (format sout "namespace ~a { ~%" (tags:namespace% class-tag))
            (format sout "  core::Symbol_sp ~a::static_class_symbol;~%" (tags:name% class-tag))
            (format sout "  core::Class_sp ~a::static_class;~%" (tags:name% class-tag))
            (format sout "  int ~a::static_Kind;~%" (tags:name% class-tag))
            (format sout "  gctools::smart_ptr<core::Creator_O> ~a::static_creator;~%" (tags:name% class-tag))
            (format sout "};~%")))
        (format sout "#endif // EXPOSE_STATIC_CLASS_VARIABLES~%"))
      (progn
        (format sout "#ifdef EXPOSE_METHODS~%")
        (dolist (exposed-class sorted-classes)
          (let ((class-tag (class-tag% exposed-class)))
            (format sout "namespace ~a {~%" (tags:namespace% class-tag))
            (format sout "void ~a::expose_to_clasp() {~%" (tags:name% class-tag))
            (format sout "    ~a<~a>()~%"
                    (if (typep exposed-class 'exposed-external-class)
                        "core::externalClass_"
                        "core::class_")
                    (tags:name% class-tag))
            (dolist (method (methods% exposed-class))
              (if (typep method 'expose-defmethod)
                  (let* ((lisp-name (lisp-name% method))
                         (class-name (tags:name% class-tag))
                         (method-name (method-name% method))
                         (lambda-list (lambda-list% method))
                         (declare-form (declare% method)))
                    (format sout "        .def(~a,&~a::~a,R\"lambda(~a)lambda\",R\"decl(~a)decl\")~%"
                            lisp-name
                            class-name
                            method-name
                            (if (string/= lambda-list "")
                                (format nil "(~a)" lambda-list)
                                lambda-list)
                            declare-form))
                  (let* ((lisp-name (lisp-name% method))
                         (pointer (pointer% method))
                         (lambda-list (lambda-list% method))
                         (declare-form (declare% method)))
                    (format sout "        .def(~a,~a,R\"lambda(~a)lambda\",R\"decl(~a)decl\")~%"
                            lisp-name
                            pointer
                            (if (string/= lambda-list "")
                                (format nil "(~a)" lambda-list)
                                lambda-list)
                            declare-form))))
            (format sout "     ;~%")
            (dolist (class-method (class-methods% exposed-class))
              (if (typep class-method 'expose-def-class-method)
                  (let* ((lisp-name (lisp-name% class-method))
                         (class-name (tags:name% class-tag))
                         (method-name (method-name% class-method))
                         (lambda-list (lambda-list% class-method))
                         (declare-form (declare% class-method)))
                    (format sout " expose_function(~a,true,&~a::~a,R\"lambda(~a)lambda\");~%"
                            lisp-name
                            class-name
                            method-name
                            (maybe-wrap-lambda-list lambda-list)))))
            (format sout "}~%")
            (format sout "};~%")))
        (format sout "#endif // EXPOSE_METHODS~%")))))
          
(defparameter *unique-symbols* nil)
(defparameter *symbols-by-package* nil)
(defparameter *symbols-by-namespace* nil)
(defun generate-code-for-symbols (packages-to-create symbols)
  (declare (optimize (speed 3)))
  ;; Uniqify the symbols
  (with-output-to-string (sout)
    (let ((symbols-by-package (make-hash-table :test #'equal))
          (symbols-by-namespace (make-hash-table :test #'equal))
          (index 0))
      (setq *symbols-by-package* symbols-by-package)
      (setq *symbols-by-namespace* symbols-by-namespace)
      ;; Organize symbols by package
      (dolist (symbol symbols)
        (pushnew symbol
                 (gethash (package% symbol) symbols-by-package)
                 :test #'string=
                 :key (lambda (x)
                        (c++-name% x)))
        (pushnew symbol
                 (gethash (namespace% symbol) symbols-by-namespace)
                 :test #'string=
                 :key (lambda (x)
                        (c++-name% x))))
      (progn
        (format sout "#if defined(BOOTSTRAP_PACKAGES)~%")
        (mapc (lambda (pkg)
                (format sout "{~%")
                (format sout "  std::list<std::string> use_packages = {~{ ~s~^, ~}};~%" (packages-to-use% pkg))
                (format sout "  bootStrapSymbolMap->add_package_info(~s,use_packages);~%" (name% pkg))
                (format sout "}~%"))
              packages-to-create)
        (format sout "#endif // #if defined(BOOTSTRAP_PACKAGES)~%"))
      (progn
        (format sout "#if defined(CREATE_ALL_PACKAGES)~%")
        (mapc (lambda (pkg)
                (format sout "{~%")
                (format sout "  std::list<std::string> nicknames = {~{ ~s~^, ~}};~%" (nicknames% pkg))
                (format sout "  std::list<std::string> use_packages = {};~%")
                (format sout "  std::list<std::string> shadow = {~{ ~s~^, ~}};~%" (shadow% pkg))
                (format sout "  if (!_lisp->recognizesPackage(~s)) {~%" (name% pkg) )
                (format sout "      _lisp->makePackage(~s,nicknames,use_packages,shadow);~%" (name% pkg))
                (format sout "  }~%")
                (format sout "}~%"))
              packages-to-create)
        (mapc (lambda (pkg)
                (when (packages-to-use% pkg)
                  (mapc (lambda (use)
                          (format sout "  gc::As<core::Package_sp>(_lisp->findPackage(~s))->usePackage(gc::As<core::Package_sp>(_lisp->findPackage(~s)));~%" (name% pkg) use))
                        (packages-to-use% pkg))))
              packages-to-create)
        (format sout "#endif~%"))
      (let ((symbol-count 0)
            (symbol-index 0))
        (maphash (lambda (key symbols)
                   (setf symbol-count (+ symbol-count (length symbols))))
                 symbols-by-package)
        (progn
          (format sout "#if defined(DECLARE_ALL_SYMBOLS)~%")
          (format sout "int global_symbol_count = ~d;~%" symbol-count)
          (format sout "core::Symbol_sp global_symbols[~d];~%" symbol-count)
          (maphash (lambda (namespace namespace-symbols)
                     (format sout "namespace ~a {~%" namespace)
                     (dolist (symbol namespace-symbols)
                       (format sout "core::Symbol_sp& _sym_~a = global_symbols[~d];~%"
                               (c++-name% symbol)
                               (1- (incf symbol-index))))
                     (format sout "} // namespace ~a~%" namespace))
                   symbols-by-namespace)
          (format sout "#endif~%"))
        (progn
          (format sout "#if defined(EXTERN_ALL_SYMBOLS)~%")
          (maphash (lambda (namespace namespace-symbols)
                     (format sout "namespace ~a {~%" namespace)
                     (dolist (symbol namespace-symbols)
                       (format sout "extern core::Symbol_sp& _sym_~a;~%"
                               (c++-name% symbol)))
                     (format sout "} // namespace ~a~%" namespace))
                   symbols-by-namespace)
          (format sout "#endif // EXTERN_ALL_SYMBOLS~%"))
        (let ((helpers (make-hash-table :test #'equal))
              (index 0))
          (format sout "#if defined(ALLOCATE_ALL_SYMBOLS_HELPERS)~%")
          (dolist (p packages-to-create)
            (maphash (lambda (namespace namespace-symbols)
                       (dolist (symbol namespace-symbols)
                         (when (string= (name% p) (package-str% symbol))
                           (let ((helper-name (format nil "maybe_allocate_one_symbol_~d_helper" (incf index)))
                                 (symbol-name (format nil "~a::_sym_~a" namespace (c++-name% symbol))))
                             (setf (gethash symbol-name helpers) helper-name)
                             (format sout "NOINLINE void ~a(core::BootStrapCoreSymbolMap* symbols) {~%" helper-name)
                             (format sout " ~a = symbols->maybe_allocate_unique_symbol(\"~a\",core::lispify_symbol_name(~s), ~a,~a);~%"
                                     symbol-name
                                     (package-str% symbol)
                                     (lisp-name% symbol)
                                     (if (exported% symbol) "true" "false")
                                     (if (shadow% symbol) "true" "false"))
                             (format sout "}~%")))))
                     symbols-by-namespace))
          (format sout "#endif // ALLOCATE_ALL_SYMBOLS_HELPERS~%")
          (format sout "#if defined(ALLOCATE_ALL_SYMBOLS)~%")
          (maphash (lambda (symbol-name helper-name)
                     (declare (ignore symbol-name))
                     (format sout " ~a(symbols);~%" helper-name))
                   helpers)
          (format sout "#endif // ALLOCATE_ALL_SYMBOLS~%"))
        #+(or)(progn
                (format sout "#if defined(GARBAGE_COLLECT_ALL_SYMBOLS)~%")
                (maphash (lambda (namespace namespace-symbols)
                           (dolist (symbol namespace-symbols)
                             (format sout "SMART_PTR_FIX(~a::_sym_~a);~%"
                                     namespace
                                     (c++-name% symbol))))
                         symbols-by-namespace)
                (format sout "#endif~% // defined(GARBAGE_COLLECT_ALL_SYMBOLS~%"))
        (progn
          (maphash (lambda (package package-symbols)
                     (format sout "#if defined(~a_SYMBOLS)~%" package)
                     (dolist (symbol package-symbols)
                       (format sout "DO_SYMBOL(~a,_sym_~a,~d,~a,~s,~a);~%"
                               (namespace% symbol)
                               (c++-name% symbol)
                               index
                               (package% symbol)
                               (lisp-name% symbol)
                               (if (typep symbol 'expose-internal-symbol)
                                   "false"
                                   "true"))
                       (incf index))
                     (format sout "#endif // ~a_SYMBOLS~%" package))
                   symbols-by-package))))))

(defun generate-code-for-enums (enums)
  (declare (optimize (speed 3)))
  ;; Uniqify the symbols
  (with-output-to-string (sout)
    (format sout "#ifdef ALL_ENUMS~%")
    (dolist (e enums)
      (format sout "core::enum_<~a>(~a,~s)~%"
              (type% (begin-enum% e))
              (symbol% (begin-enum% e))
              (description% (begin-enum% e)))
      (dolist (value (values% e))
        (format sout "  .value(~a,~a)~%"
                (symbol% value)
                (value% value)))
      (format sout ";~%"))
    (format sout "#endif //ifdef ALL_ENUMS~%")))

(defun generate-code-for-initializers (initializers)
  (declare (optimize (speed 3)))
  (let ((initializers-by-namespace (make-hash-table :test #'equal)))
    (dolist (i initializers)
      (push i (gethash (namespace% i) initializers-by-namespace)))
    (with-output-to-string (sout)
      (format sout "#ifdef ALL_INITIALIZERS_EXTERN~%")
      (maphash (lambda (ns init-list)
                 (format sout "namespace ~a {~%" ns)
                 (dolist (ii init-list)
                   (format sout "   extern void ~a();~%" (function-name% ii)))
                 (format sout "};~%"))
               initializers-by-namespace)
      (format sout "#endif // ALL_INITIALIZERS_EXTERN~%")
      (format sout "#ifdef ALL_INITIALIZERS_CALLS~%")
      (maphash (lambda (ns init-list)
                 (dolist (ii init-list)
                   (format sout "    ~a::~a();~%" (namespace% ii) (function-name% ii))))
               initializers-by-namespace)
      (format sout "#endif // ALL_INITIALIZERS_CALL~%"))))

(defun maybe-relative (dir)
  (cond
    ((eq (car dir) :absolute) (list* :relative (cdr dir)))
    ((eq (car dir) :relative) dir)
    (t (error "How do I convert this to a relative directory path: ~a" dir))))

(defun write-if-changed (code main-path app-relative)
  (let ((pn (merge-pathnames
             (make-pathname :name (pathname-name app-relative)
                            :type (pathname-type app-relative)
                            :directory (maybe-relative (pathname-directory app-relative)))
             (pathname main-path))))
    (ensure-directories-exist pn)
    (let ((data-in-file (when (probe-file pn)
                          (with-open-file (stream pn :direction :input)
                            (let ((data (make-string (file-length stream))))
                              (read-sequence data stream)
                              data)))))
      (if (string= data-in-file code)
          (format t   "| -------- | There are no changes to ~a.~%" pn)
          (progn
            (format t "| UPDATING | There are changes to ~a.~%" pn)
            (with-open-file (stream pn :direction :output :if-exists :supersede)
              (write-sequence code stream)))))))

(defun safe-app-config (key app-config)
  (let ((value (gethash key app-config)))
    (unless value
      (error "Could not get key: ~s from app-config" key))
    value))

(defun generate-code (packages-to-create functions symbols classes enums initializers build-path app-config)
  (let ((init-functions (generate-code-for-init-functions functions))
        (init-classes-and-methods (generate-code-for-init-classes-and-methods classes))
        (source-info (generate-code-for-source-info functions classes))
        (symbol-info (generate-code-for-symbols packages-to-create symbols))
        (enum-info (generate-code-for-enums enums))
        (initializers-info (generate-code-for-initializers initializers)))
    (write-if-changed init-functions build-path (safe-app-config :init_functions_inc_h app-config))
    (write-if-changed init-classes-and-methods build-path (safe-app-config :init_classes_inc_h app-config))
    (write-if-changed source-info build-path (safe-app-config :source_info_inc_h app-config))
    (write-if-changed symbol-info build-path (safe-app-config :symbols_scraped_inc_h app-config))
    (write-if-changed enum-info build-path (safe-app-config :enum_inc_h app-config))
    (write-if-changed initializers-info build-path (safe-app-config :initializers_inc_h app-config))
    (multiple-value-bind (direct-call-c-code direct-call-cl-code c-code-info cl-code-info)
        (generate-code-for-direct-call-functions functions)
      (write-if-changed direct-call-c-code build-path (safe-app-config :c_wrappers app-config))
      (write-if-changed direct-call-cl-code build-path (safe-app-config :lisp_wrappers app-config))
      (write-if-changed c-code-info build-path (merge-pathnames (make-pathname :type "txt") (safe-app-config :c_wrappers app-config)))
      (write-if-changed cl-code-info build-path (merge-pathnames (make-pathname :type "txt") (safe-app-config :lisp_wrappers app-config))))))
