
// (c) COPYRIGHT URI/MIT 1994-1996
// Please read the full copyright statement in the file COPYRIGH.  
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

    strcpy(_program, pgm);
}

Error::Error(const Error &copy_from)
    : _error_code(copy_from._error_code),
      _error_message(copy_from._error_message),
      _program_type(copy_from._program_type), _program(0)
{
    _program = new char[strlen(copy_from._program) + 1];
    strcpy(_program, copy_from._program);
}    

Error::~Error()
{
    delete _program;
}

Error &
Error::operator=(const Error &rhs)
{
    if (&rhs == this)		// are they identical?
	return *this;
    else {
	_error_code = rhs._error_code;
	_error_message = rhs._error_message;
	_program_type = rhs._program_type;

	_program = new char[strlen(rhs._program) + 1];
	strcpy(_program, rhs._program);

	return *this;
    }
}

// To be a valid, an Error object must either be: 1) empty, 2) contain a
// message and a program or 3) contain only a message. Since the program is
// optional, there is no need to test for it here. 
//
// NB: This mfunc does not test for malformed messages or programs - ones
// where the code is defined but not the `data'.

bool
Error::OK()
{
    bool empty = ((_error_code == undefined_error) 
		  && (_error_message == "")
		  && (_program_type == undefined_prog_type) 
		  && (_program == 0));

    bool message = ((_error_code != undefined_error) 
		    && (_error_message != ""));

    // bool program = ((_program_type != undefined_prog_type) 
    //                 && (_program != 0))

    return empty || message;
}

bool
Error::parse(FILE *fp)
{
    if (!fp) {
	cerr << "Error::parse: Null input stream" << endl;
	return false;
    }

    Errorrestart(fp);

    parser_arg arg(this);

    bool status = Errorparse(&arg) == 0;

    fclose(fp);

    // need to check the arg status because the parser may have recovered
    // from an error (and thus returned true).
    return status && arg.status() && OK();
}
    
void
Error::print(ostream &os = cout)
{
    if (!OK()) {
	cerr << "Bad Error object" << endl;
	return;
    }

    os << "Error {" << endl;

    if (_error_code != undefined_error) {
	os << "    " << "code = " << _error_code << ";" << endl;
	os << "    " << "message = " << _error_message << ";" << endl;

	if (_program_type != undefined_prog_type) {
	    os << "    " << "program_type = " << _program_type << ";" << endl;
	    os << "    " << "program = " << _program << ";" << endl;
	}
    }    

    os << "};" << endl;
}

ErrorCode
Error::error_code(ErrorCode ec = undefined_error)
{
    if (ec == undefined_error)
	return _error_code;
    else {
	_error_code = ec;
	return _error_code;
    }
}

String
Error::error_message(String msg = "")
{
    if (msg == "")
	return String(_error_message);
    else {
	_error_message = msg;
	return String (_error_message);
    }
}

// Check the DISPLAY environment variable, if defined, use X11. If not use
// stderr. 
void
Error::display_message()
{
#if TCLTK
    if (getenv("DISPLAY"))
	tk_display_message(_error_message);
    else
#else
	cerr << _error_message << endl;
#endif
}

ProgramType
Error::program_type(ProgramType pt = undefined_prog_type)
{
    if (pt == undefined_prog_type)
	return _program_type;
    else {
	_program_type = pt;
	return _program_type;
    }
}

char *
Error::program(char *pgm = 0)
{
    if (pgm == 0)
	return _program;
    else {
	_program = new char[strlen(pgm) + 1];
	strcpy(_program, pgm);
	return _program;
    }
}

String
Error::correct_error()
{
    if (OK())
	display_message();

    return String("");
}
