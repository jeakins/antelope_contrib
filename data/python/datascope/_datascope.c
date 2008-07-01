/*
 *   Copyright (c) 2007-2008 Lindquist Consulting, Inc.
 *   All rights reserved. 
 *                                                                     
 *   Written by Dr. Kent Lindquist, Lindquist Consulting, Inc. 
 *
 *   This software is licensed under the New BSD license: 
 *
 *   Redistribution and use in source and binary forms,
 *   with or without modification, are permitted provided
 *   that the following conditions are met:
 *   
 *   * Redistributions of source code must retain the above
 *   copyright notice, this list of conditions and the
 *   following disclaimer.
 *   
 *   * Redistributions in binary form must reproduce the
 *   above copyright notice, this list of conditions and
 *   the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *   
 *   * Neither the name of Lindquist Consulting, Inc. nor
 *   the names of its contributors may be used to endorse
 *   or promote products derived from this software without
 *   specific prior written permission.

 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 *   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *   THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 *   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "Python.h"
#include "db.h"
#include "dbxml.h"
#include "tr.h"

static Arr *Hooks = 0;

#ifdef __APPLE__

/* The scaffold below works around a problem on Darwin */

char **environ;
char *__progname = "Python";

#endif

static PyObject *python_dbinvalid( PyObject *self, PyObject *args );
static PyObject *python_dbopen( PyObject *self, PyObject *args );
static PyObject *python_dbclose( PyObject *self, PyObject *args );
static PyObject *python_dbfree( PyObject *self, PyObject *args );
static PyObject *python_dbdelete( PyObject *self, PyObject *args );
static PyObject *python_dbmark( PyObject *self, PyObject *args );
static PyObject *python_dbcrunch( PyObject *self, PyObject *args );
static PyObject *python_dbdestroy( PyObject *self, PyObject *args );
static PyObject *python_dbtruncate( PyObject *self, PyObject *args );
static PyObject *python_dblookup( PyObject *self, PyObject *args );
static PyObject *python_dbsort( PyObject *self, PyObject *args );
static PyObject *python_dbsubset( PyObject *self, PyObject *args );
static PyObject *python_dblist2subset( PyObject *self, PyObject *args );
static PyObject *python_dbseparate( PyObject *self, PyObject *args );
static PyObject *python_dbsever( PyObject *self, PyObject *args );
static PyObject *python_dbjoin( PyObject *self, PyObject *args );
static PyObject *python_dbnojoin( PyObject *self, PyObject *args );
static PyObject *python_dbtheta( PyObject *self, PyObject *args );
static PyObject *python_dbunjoin( PyObject *self, PyObject *args );
static PyObject *python_dbgroup( PyObject *self, PyObject *args );
static PyObject *python_dbungroup( PyObject *self, PyObject *args );
static PyObject *python_dbprocess( PyObject *self, PyObject *args );
static PyObject *python_dbgetv( PyObject *self, PyObject *args );
static PyObject *python_dbaddv( PyObject *self, PyObject *args );
static PyObject *python_dbputv( PyObject *self, PyObject *args );
static PyObject *python_dbaddnull( PyObject *self, PyObject *args );
static PyObject *python_dbextfile( PyObject *self, PyObject *args );
static PyObject *python_dbex_eval( PyObject *self, PyObject *args );
static PyObject *python_dbquery( PyObject *self, PyObject *args );
static PyObject *python_dbmatches( PyObject *self, PyObject *args );
static PyObject *python_dbfind( PyObject *self, PyObject *args );
static PyObject *python_db2xml( PyObject *self, PyObject *args );
static PyObject *python_dbnextid( PyObject *self, PyObject *args );
static PyObject *python_dbtmp( PyObject *self, PyObject *args );
static PyObject *python_dbcreate( PyObject *self, PyObject *args );
static PyObject *python_trloadchan( PyObject *self, PyObject *args );
static PyObject *python_trsample( PyObject *self, PyObject *args );
static PyObject *python_trfilter( PyObject *self, PyObject *args );
static PyObject *python_trdata( PyObject *self, PyObject *args );
static PyObject *python_trcopy( PyObject *self, PyObject *args );
static PyObject *python_trsplice( PyObject *self, PyObject *args );
static PyObject *python_trsplit( PyObject *self, PyObject *args );
static PyObject *python_trfree( PyObject *self, PyObject *args );
static PyObject *python_trdestroy( PyObject *self, PyObject *args );
static PyObject *python_trtruncate( PyObject *self, PyObject *args );
static PyObject *python_trlookup_segtype( PyObject *self, PyObject *args );
static PyObject *python_trwfname( PyObject *self, PyObject *args );
static void add_datascope_constants( PyObject *mod );
static int parse_to_Dbptr( PyObject *obj, void *addr );
PyMODINIT_FUNC init_datascope( void );

static struct PyMethodDef _datascope_methods[] = {
	{ "_dbopen",   	python_dbopen,   	METH_VARARGS, "Open Datascope database" },
	{ "_dbclose",  	python_dbclose,   	METH_VARARGS, "Close a Datascope database" },
	{ "_dbfree",  	python_dbfree,   	METH_VARARGS, "Free Datascope memory" },
	{ "_dbdelete", 	python_dbdelete,   	METH_VARARGS, "Delete Rows from tables" },
	{ "_dbmark", 	python_dbmark,   	METH_VARARGS, "Mark  rows in tables" },
	{ "_dbcrunch", 	python_dbcrunch,   	METH_VARARGS, "Delete marked rows from tables" },
	{ "_dbdestroy",	python_dbdestroy,	METH_VARARGS, "Completely eliminate every table in a database" },
	{ "_dbtruncate", python_dbtruncate,	METH_VARARGS, "Truncate a database table" },
	{ "_dblookup", 	python_dblookup, 	METH_VARARGS, "Lookup Datascope indices" },
	{ "_dbsort",   	python_dbsort,   	METH_VARARGS, "Sort Datascope table" },
	{ "_dbsubset", 	python_dbsubset, 	METH_VARARGS, "Subset Datascope table" },
	{ "_dblist2subset", python_dblist2subset, METH_VARARGS, "Convert a list of records to a database subset" },
	{ "_dbseparate", python_dbseparate, 	METH_VARARGS, "Extract a subset of a base table from a joined view" },
	{ "_dbsever", 	python_dbsever,		METH_VARARGS, "Remove a table from a joined view" },
	{ "_dbprocess",	python_dbprocess, 	METH_VARARGS, "Run a series of database operations" },
	{ "_dbjoin",   	python_dbjoin,   	METH_VARARGS, "Join Datascope tables" },
	{ "_dbnojoin", 	python_dbnojoin,   	METH_VARARGS, "Return records which don't join" },
	{ "_dbtheta",  	python_dbtheta,   	METH_VARARGS, "Theta-join Datascope tables" },
	{ "_dbunjoin", 	python_dbunjoin,   	METH_VARARGS, "Create new tables from a joined table" },
	{ "_dbgroup",  	python_dbgroup,   	METH_VARARGS, "Group a sorted table" },
	{ "_dbungroup",	python_dbungroup,   	METH_VARARGS, "Ungroup a grouped table" },
	{ "_dbinvalid", python_dbinvalid,   	METH_VARARGS, "Create an invalid database pointer" },
	{ "_dbgetv",    python_dbgetv,   	METH_VARARGS, "Retrieve values from a database row" },
	{ "_dbaddv",    python_dbaddv,   	METH_VARARGS, "Add records to a database table" },
	{ "_dbputv",    python_dbputv,   	METH_VARARGS, "Write fields to a database table" },
	{ "_dbaddnull", python_dbaddnull,   	METH_VARARGS, "Add a new, null row to a database table" },
	{ "_dbextfile", python_dbextfile,   	METH_VARARGS, "Retrieve an external file name from a database row" },
	{ "_dbex_eval", python_dbex_eval,   	METH_VARARGS, "Evaluate a database expression" },
	{ "_dbquery",   python_dbquery,   	METH_VARARGS, "Get ancillary information about a database" },
	{ "_dbmatches", python_dbmatches,   	METH_VARARGS, "Find matching records in second table" },
	{ "_dbfind",    python_dbfind,   	METH_VARARGS, "Search for matching record in table" },
	{ "_db2xml",    python_db2xml,   	METH_VARARGS, "Convert a database view to XML" },
	{ "_dbnextid",  python_dbnextid,   	METH_VARARGS, "Generate a unique id from the lastid table" },
	{ "_dbtmp",     python_dbtmp,  		METH_VARARGS, "Create a new database descriptor" },
	{ "_dbcreate",  python_dbcreate,   	METH_VARARGS, "Create a temporary database" },
	{ "_trloadchan", python_trloadchan,	METH_VARARGS, "Read channel waveform data" },
	{ "_trsample",  python_trsample,	METH_VARARGS, "Return channel waveform data" },
	{ "_trfilter",  python_trfilter,	METH_VARARGS, "Apply time-domain filters to waveform data" },
	{ "_trdata",	python_trdata,		METH_VARARGS, "Extract data points from trace table record" },
	{ "_trcopy",	python_trcopy,		METH_VARARGS, "Make copy of a trace table including the trace data" },
	{ "_trsplice",	python_trsplice,	METH_VARARGS, "Splice together data segments" },
	{ "_trsplit",	python_trsplit,		METH_VARARGS, "Split data segments which contain marked gaps" },
	{ "_trfree",	python_trfree,		METH_VARARGS, "Free memory buffers and clear trace object tables" },
	{ "_trdestroy",	python_trdestroy,	METH_VARARGS, "Close a trace database, cleaning up memory and files" },
	{ "_trtruncate", python_trtruncate,	METH_VARARGS, "Truncate a tr database table" },
	{ "_trlookup_segtype", python_trlookup_segtype,	METH_VARARGS, "Lookup segtype in segtype table" },
	{ "_trwfname", 	python_trwfname,	METH_VARARGS, "Generate waveform file names" },
	{ NULL, NULL, 0, NULL }
};

static PyObject *
Dbptr2PyObject( Dbptr db )
{
	return Py_BuildValue( "[iiii]", db.database, db.table, db.field, db.record );
}

static PyObject *
Dbvalue2PyObject( Dbvalue value, int type )
{
	PyObject *obj;
	
	switch( type ) {

	case dbBOOLEAN:

		if( value.i ) {

			Py_INCREF( Py_True );

			obj = Py_True;

		} else {

			Py_INCREF( Py_False );

			obj = Py_False;
		}

		break;

	case dbINTEGER:
	case dbYEARDAY:
		
		obj = Py_BuildValue( "i", value.i );

		break;

	case dbREAL:
	case dbTIME:
		
		obj = Py_BuildValue( "d", value.i );

		break;

	case dbSTRING:
		
		obj = Py_BuildValue( "s", value.t );

		free( value.t );

		break;

	default:

		obj = NULL;

		break;
	}

	return obj;
}

static int 
PyObject2Dbvalue( PyObject *obj, int type, Dbvalue *value )
{
	int	retcode = 0;

	switch( type ) {

	case dbDBPTR:

		if( parse_to_Dbptr( obj, &value ) ) {
		
			retcode = 1;

		} else {

			retcode = 0;
		}

		break;

	case dbSTRING:

		if( PyString_Check( obj ) ) {

			value->t = PyString_AsString( obj );
	
			retcode = 1;

		} else { 

			retcode = 0;
		}

		break;

	case dbBOOLEAN:

		if( obj == Py_False ) {

			value->i = 0;

			retcode = 1;

		} else if( obj == Py_True ) {

			value->i = -1;

			retcode = 1;

		} else if( PyInt_Check( obj ) ) {

			value->i = PyInt_AsLong( obj );
	
			retcode = 1;

		} else { 

			retcode = 0;
		}

		break;

	case dbINTEGER:
	case dbYEARDAY:

		if( PyInt_Check( obj ) ) {

			value->i = PyInt_AsLong( obj );
	
			retcode = 1;

		} else { 

			retcode = 0;
		}

		break;

	case dbREAL:

		if( PyFloat_Check( obj ) ) {

			value->d = PyFloat_AsDouble( obj );
	
			retcode = 1;

		} else if( PyInt_Check( obj ) ) { 

			value->d = (double) PyInt_AsLong( obj );
	
			retcode = 1;

		} else { 

			retcode = 0;
		}

		break;

	case dbTIME:

		if( PyString_Check( obj ) ) {

			value->d = str2epoch( PyString_AsString( obj ) );
	
			retcode = 1;

		} else if( PyFloat_Check( obj ) ) {

			value->d = PyFloat_AsDouble( obj );
	
			retcode = 1;

		} else { 

			retcode = 0;
		}

		break;

	default:

		retcode = 0;

		break;
	}

	return retcode;
}

static PyObject *
strtbl2PyObject( Tbl *atbl ) 
{
	PyObject *obj;
	int	i;

	if( atbl == NULL ) {

		return NULL;
	} 

	obj = PyTuple_New( maxtbl( atbl ) );

	for( i = 0; i < maxtbl( atbl ); i++ ) {

		PyTuple_SetItem( obj, i, PyString_FromString( gettbl( atbl, i ) ) );
	}

	return obj;
}

static PyObject *
strarr2PyObject( Arr *anarr ) 
{
	PyObject *obj;
	Tbl	*keys;
	char	*key;
	char	*value;
	int	i;

	if( anarr == NULL ) {

		return NULL;
	} 

	keys = keysarr( anarr );

	obj = PyDict_New();

	for( i = 0; i < maxtbl( keys ); i++ ) {

		key = gettbl( keys, i );

		value = getarr( anarr, key );

		PyDict_SetItem( obj, PyString_FromString( key ), PyString_FromString( value ) );
	}

	freetbl( keys, 0 );

	return obj;
}

static PyObject *
inttbl2PyObject( Tbl *atbl )
{
	PyObject *obj;
	int	i;

	if( atbl == NULL ) {

		return NULL;
	}
	
	obj = PyTuple_New( maxtbl( atbl ) );

	for( i = 0; i < maxtbl( atbl ); i++ ) {

		PyTuple_SetItem( obj, i, PyInt_FromLong( (long) gettbl( atbl, i ) ) );
	}

	return obj;
}

static int
parse_to_Dbptr( PyObject *obj, void *addr )
{
	Dbptr	*db = (Dbptr *) addr;

	if( PyList_Check( obj ) &&
	    PyList_Size( obj ) == 4 &&
	    PyInt_Check( PyList_GetItem( obj, 0 ) ) &&
	    PyInt_Check( PyList_GetItem( obj, 1 ) ) &&
	    PyInt_Check( PyList_GetItem( obj, 2 ) ) &&
	    PyInt_Check( PyList_GetItem( obj, 3 ) ) ) {

		db->database = (int) PyInt_AsLong( PyList_GetItem( obj, 0 ) );
		db->table    = (int) PyInt_AsLong( PyList_GetItem( obj, 1 ) );
		db->field    = (int) PyInt_AsLong( PyList_GetItem( obj, 2 ) );
		db->record   = (int) PyInt_AsLong( PyList_GetItem( obj, 3 ) );

		return 1;

	} else {

		PyErr_SetString( PyExc_TypeError, 
			"Dbptr is not a Dbptr object or 4-integer list" );
	}

	return 0;
}

static int
parse_from_Boolean( PyObject *obj, void *addr )
{
	int	*value = (int *) addr;

	if( obj == Py_False ) {

		*value = 0;

		return 1;

	} else if( obj == Py_True ) {

		*value = -1;

		return 1;

	} else {

		PyErr_SetString( PyExc_TypeError, 
			"Attempt to coerce non-Boolean value into Boolean" );
	}

	return 0;
}

static int
parse_to_inttbl( PyObject *obj, void *addr )
{
	Tbl	**atbl = (Tbl **) addr;
	PyObject *seqobj;
	int	nitems = 0;
	int	iitem;
	long	along;
	char	errmsg[STRSZ];

	if( obj == Py_None ) {

		*atbl = 0;

		return 1;
	} 

	if( PyInt_Check( obj ) ) {

		*atbl = newtbl( 1 );

		pushtbl( *atbl, (void *) PyInt_AsLong( obj ) );

		return 1;
	}

	if( ! PySequence_Check( obj ) ) {

		PyErr_SetString( PyExc_TypeError, 
			"Attempt to convert sequence to table of integers failed: input argument is not a sequence" );

		return 0;
	}

	nitems = PySequence_Size( obj );

	*atbl = newtbl( nitems );

	for( iitem = 0; iitem < nitems; iitem++ ) {
		
		seqobj = PySequence_GetItem( obj, iitem );

		if( ! seqobj ) {

			freetbl( *atbl, 0 );

			*atbl = 0;

			sprintf( errmsg, 
				"Attempt to convert sequence to table of strings failed: "
				"failed to extract item %d (counting from 0)", iitem );

			PyErr_SetString( PyExc_TypeError, errmsg );

			return 0;
		}

		if( ! PyInt_Check( seqobj ) ) {

			freetbl( *atbl, 0 );

			*atbl = 0;

			sprintf( errmsg, 
				"Attempt to convert sequence to table of strings failed: "
				"item %d (counting from 0) is not an integer", iitem );

			PyErr_SetString( PyExc_TypeError, errmsg );

			return 0;
		}

		if( ( along = PyInt_AsLong( seqobj ) ) == -1 && PyErr_Occurred() ) {

			freetbl( *atbl, 0 );

			*atbl = 0;

			sprintf( errmsg, 
				"Attempt to convert sequence to table of strings failed: "
				"conversion of item %d (counting from 0) to long failed", iitem );

			PyErr_SetString( PyExc_TypeError, errmsg );

			return 0;
		}

		pushtbl( *atbl, (void *) along );
	}

	return 1;
}

static int
parse_to_strtbl( PyObject *obj, void *addr )
{
	Tbl	**atbl = (Tbl **) addr;
	PyObject *seqobj;
	int	nitems = 0;
	int	iitem;
	char	*astring;
	char	errmsg[STRSZ];

	if( obj == Py_None ) {

		*atbl = 0;

		return 1;
	} 

	if( PyString_Check( obj ) ) {

		*atbl = strtbl( PyString_AsString( obj ), NULL );

		return 1;
	}

	if( ! PySequence_Check( obj ) ) {

		PyErr_SetString( PyExc_TypeError, 
			"Attempt to convert sequence to table of strings failed: input argument is not a sequence" );

		return 0;
	}

	nitems = PySequence_Size( obj );

	*atbl = newtbl( nitems );

	for( iitem = 0; iitem < nitems; iitem++ ) {
		
		seqobj = PySequence_GetItem( obj, iitem );

		if( ! seqobj ) {

			freetbl( *atbl, 0 );

			*atbl = 0;

			sprintf( errmsg, 
				"Attempt to convert sequence to table of strings failed: "
				"failed to extract item %d (counting from 0)", iitem );

			PyErr_SetString( PyExc_TypeError, errmsg );

			return 0;
		}

		if( ! PyString_Check( seqobj ) ) {

			freetbl( *atbl, 0 );

			*atbl = 0;

			sprintf( errmsg, 
				"Attempt to convert sequence to table of strings failed: "
				"item %d (counting from 0) is not a string", iitem );

			PyErr_SetString( PyExc_TypeError, errmsg );

			return 0;
		}

		if( ( astring = PyString_AsString( seqobj ) ) == NULL ) {

			freetbl( *atbl, 0 );

			*atbl = 0;

			sprintf( errmsg, 
				"Attempt to convert sequence to table of strings failed: "
				"conversion of item %d (counting from 0) to string failed", iitem );

			PyErr_SetString( PyExc_TypeError, errmsg );

			return 0;
		}

		pushtbl( *atbl, astring );
	}

	return 1;
}

static PyObject *
python_dbopen( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbopen(dbname, perm)\n";
	char	*dbname;
	char	*perm;
	Dbptr	db;
	int	rc;

	if( ! PyArg_ParseTuple( args, "ss", &dbname, &perm ) ) {
		
		PyErr_SetString( PyExc_RuntimeError, usage );

		return NULL;
	}

	rc = dbopen( dbname, perm, &db );

	if( rc < 0 ) {

		return Py_BuildValue( "" );

	} else {

		return Dbptr2PyObject( db );
	}
}

static PyObject *
python_dbclose( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbclose(db)\n";
	Dbptr	db;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbclose( db );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error closing database" );

		return NULL;
	}

	return Py_BuildValue( "" );
}

static PyObject *
python_dbdelete( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbdelete(db)\n";
	Dbptr	db;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbdelete( db );

	if( rc != 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error deleting database rows" );

		return NULL;
	}

	return Py_BuildValue( "" );
}

static PyObject *
python_dbmark( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbmark(db)\n";
	Dbptr	db;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbmark( db );

	if( rc != 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error deleting database rows" );

		return NULL;
	}

	return Py_BuildValue( "" );
}

static PyObject *
python_dbcrunch( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbcrunch(db)\n";
	Dbptr	db;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbcrunch( db );

	if( rc != 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error deleting database rows" );

		return NULL;
	}

	return Py_BuildValue( "" );
}

static PyObject *
python_dbtruncate( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbtruncate(db, nrecords)\n";
	Dbptr	db;
	int	rc;
	int	nrecords;

	if( ! PyArg_ParseTuple( args, "O&i", parse_to_Dbptr, &db, &nrecords ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbtruncate( db, nrecords );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbtruncate error" );

		return NULL;
	} 

	return Py_BuildValue( "" );
}

static PyObject *
python_dbdestroy( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbdestroy(tr)\n";
	Dbptr	db;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbdestroy( db );

	if( rc != 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error destroying database" );

		return NULL;
	}

	return Py_BuildValue( "" );
}

static PyObject *
python_dbtmp( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbtmp(schema)\n";
	Dbptr	db;
	char	*schema = 0;   

	if( ! PyArg_ParseTuple( args, "s", &schema ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dbtmp( schema );

	return Dbptr2PyObject( db );
}

static PyObject *
python_dbcreate( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbcreate(filename, schema, dbpath, description, detail)\n";
	char	*filename = 0;   
	char	*schema = 0;   
	char	*dbpath = 0;   
	char	*description = 0;   
	char	*detail = 0;   
	int	rc;

	if( ! PyArg_ParseTuple( args, "sszzz", &filename, &schema, &dbpath, &description, &detail) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbcreate( filename, schema, dbpath, description, detail );

	if( rc != 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error creating database" );

		return NULL;
	}

	return Py_BuildValue( "" );
}

static PyObject *
python_dbfree( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbfree(db)\n";
	Dbptr	db;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbfree( db );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error freeing datascope memory" );

		return NULL;
	}

	return Py_BuildValue( "" );
}

static PyObject *
python_dblookup( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dblookup(db, database, table, field, record)\n";
	Dbptr	db;
	char	*database = 0;
	char	*table = 0;
	char	*field = 0;
	char	*record = 0;

	if( ! PyArg_ParseTuple( args, "O&ssss", parse_to_Dbptr, &db, 
				      &database, &table, &field, &record ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dblookup( db, database, table, field, record );

	return Dbptr2PyObject( db );
}

static PyObject *
python_dbsubset( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbsubset(db, expr, name)\n";
	Dbptr	db;
	char	*expr = 0;
	char	*name = 0;

	if( ! PyArg_ParseTuple( args, "O&sz", parse_to_Dbptr, &db, &expr, &name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dbsubset( db, expr, name );

	return Dbptr2PyObject( db );
}

static PyObject *
python_dblist2subset( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dblist2subset(db, list)\n";
	Dbptr	db;
	Tbl	*list;

	if( ! PyArg_ParseTuple( args, "O&O&", parse_to_Dbptr, &db, parse_to_inttbl, &list ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dblist2subset( db, list );
	
	return Dbptr2PyObject( db );
}

static PyObject *
python_dbseparate( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbseparate(db, tablename)\n";
	PyObject *obj;
	Dbptr	db;
	char	*tablename = 0;

	if( ! PyArg_ParseTuple( args, "O&s", parse_to_Dbptr, &db, &tablename ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dbseparate( db, tablename );

	if( db.table < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbseparate failed" );
	
		obj = NULL;

	} else {

		obj = Dbptr2PyObject( db );
	}

	return obj;
}

static PyObject *
python_dbsever( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbsever(db, tablename, name)\n";
	PyObject *obj;
	Dbptr	db;
	char	*tablename = 0;
	char	*view_name = 0;

	if( ! PyArg_ParseTuple( args, "O&sz", parse_to_Dbptr, &db, &tablename, &view_name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dbsever( db, tablename, view_name );

	if( db.table < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbsever failed" );
	
		obj = NULL;

	} else {

		obj = Dbptr2PyObject( db );
	}

	return obj;
}

static PyObject *
python_dbex_eval( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbex_eval(db, expr)\n";
	PyObject *obj;
	Dbptr	db;
	Dbvalue value;
	char	*expr = 0;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&s", parse_to_Dbptr, &db, &expr ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbex_evalstr( db, expr, 0, &value );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbex_evalstr failed" );
	
		obj = NULL;

	} else {

		obj = Dbvalue2PyObject( value, rc );
	}

	return obj;
}

static PyObject *
python_dbextfile( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbextfile(db, tablename)\n";
	PyObject *obj;
	Dbptr	db;
	char	*tablename = 0;
	char	filename[FILENAME_MAX];
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&z", parse_to_Dbptr, &db, &tablename ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbextfile( db, tablename, filename );

	obj = Py_BuildValue( "s", filename );

	return obj;
}

static PyObject *
python_dbfind( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbfind(db, expr, first, reverse)\n";
	Dbptr	db;
	char	*expr;
	int	first;
	int	reverse = 0;
	int	flags;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&siO&", parse_to_Dbptr, &db, &expr, &first, parse_from_Boolean, &reverse ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	if( reverse ) {

		flags++;
	} 
	
	db.record = first;

	rc = dbfind( db, expr, flags, 0 );

	return Py_BuildValue( "i", rc );
}

static PyObject *
python_dbmatches( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbmatches(dbk, dbt, hookname, kpattern, tpattern)\n";
	PyObject *obj;
	Dbptr	dbk;
	Dbptr	dbt;
	char	*hookname = 0;
	Tbl	*kpattern = 0;
	Tbl 	*tpattern = 0;
	Tbl	*list = 0;
	Hook	*hook = 0;
	int	duplicate_pattern = 0;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&O&sO&O&", 
					parse_to_Dbptr, &dbk, 
					parse_to_Dbptr, &dbt,
					&hookname,
					parse_to_strtbl, &kpattern,
					parse_to_strtbl, &tpattern ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	if( kpattern != NULL && tpattern == NULL ) {

		tpattern = kpattern;

		duplicate_pattern++;
	}

	if( Hooks == 0 ) {

		Hooks = newarr( 0 );
	}

	hook = getarr( Hooks, hookname );

	rc = dbmatches( dbk, dbt, &kpattern, &tpattern, &hook, &list );

	if( kpattern != NULL ) {

		freetbl( kpattern, 0 );
	} 

	if( ( tpattern != NULL ) && ! duplicate_pattern ) {

		freetbl( tpattern, 0 );
	} 

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbmatches failed" );

		return NULL;
	}

	if( getarr( Hooks, hookname ) == NULL ) {

		setarr( Hooks, hookname, hook );
	}

	obj = inttbl2PyObject( list );

	freetbl( list, 0 );

	return obj;
}

static PyObject *
python_dbprocess( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbprocess(db, list)\n";
	Dbptr	db;
	Tbl	*list = 0;

	if( ! PyArg_ParseTuple( args, "O&O&", parse_to_Dbptr, &db, parse_to_strtbl, &list ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dbprocess( db, list, 0 );

	return Dbptr2PyObject( db );
}


static PyObject *
python_dbinvalid( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbinvalid()\n";
	Dbptr	db;

	if( ! PyArg_ParseTuple( args, "" ) ) {

		PyErr_SetString( PyExc_RuntimeError, usage );

		return NULL;
	}

	db = dbinvalid();

	return Dbptr2PyObject( db );
}

static PyObject *
python_dbsort( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbsort(db, keys, unique, reverse, name)\n";
	Dbptr	db;
	Tbl 	*keys = 0;
	char	*name = 0;
	int	flags = 0;
	int	reverse = 0;
	int	unique = 0;

	if( ! PyArg_ParseTuple( args, "O&O&O&O&z", parse_to_Dbptr, &db, 
						   parse_to_strtbl, &keys, 
						   parse_from_Boolean, &unique, 
						   parse_from_Boolean, &reverse, 
						   &name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	if( reverse ) {

		flags |= dbSORT_REVERSE;
	} 

	if( unique ) {

		flags |= dbSORT_UNIQUE;
	} 

	db = dbsort( db, keys, flags, name );

	if( keys != NULL ) {

		freetbl( keys, 0 );
	}

	return Dbptr2PyObject( db );
}

static PyObject *
python_dbjoin( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbjoin(db1, db2, pattern1, pattern2, outer, name)\n";
	Dbptr	db1;
	Dbptr	db2;
	Dbptr	dbout;
	Tbl	*pattern1 = 0;
	Tbl 	*pattern2 = 0;
	int	outer = 0;
	int	duplicate_pattern = 0;
	char	*name = 0;

	if( ! PyArg_ParseTuple( args, "O&O&O&O&O&z", parse_to_Dbptr, &db1, 
					       parse_to_Dbptr, &db2, 
					       parse_to_strtbl, &pattern1, 
					       parse_to_strtbl, &pattern2,
					       parse_from_Boolean, &outer, 
					       &name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	if( pattern1 != 0 && pattern2 == 0 ) {

		pattern2 = pattern1; 

		duplicate_pattern++;
	}

	dbout = dbjoin( db1, db2, &pattern1, &pattern2, outer, 0, name );

	if( pattern1 != NULL ) {

		freetbl( pattern1, 0 );
	}

	if( pattern2 != NULL && ! duplicate_pattern ) {

		freetbl( pattern2, 0 );
	}

	return Dbptr2PyObject( dbout );
}

static PyObject *
python_dbnojoin( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbnojoin(db1, db2, pattern1, pattern2, name)\n";
	Dbptr	db1;
	Dbptr	db2;
	Dbptr	dbout;
	Tbl	*pattern1 = 0;
	Tbl 	*pattern2 = 0;
	int	duplicate_pattern = 0;
	char	*name = 0;

	if( ! PyArg_ParseTuple( args, "O&O&O&O&z", parse_to_Dbptr, &db1, 
					       parse_to_Dbptr, &db2, 
					       parse_to_strtbl, &pattern1, 
					       parse_to_strtbl, &pattern2,
					       &name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	if( pattern1 != 0 && pattern2 == 0 ) {

		pattern2 = pattern1; 

		duplicate_pattern++;
	}

	dbout = dbnojoin( db1, db2, &pattern1, &pattern2, name );

	if( pattern1 != NULL ) {

		freetbl( pattern1, 0 );
	}

	if( pattern2 != NULL && ! duplicate_pattern ) {

		freetbl( pattern2, 0 );
	}

	return Dbptr2PyObject( dbout );
}

static PyObject *
python_dbtheta( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbtheta(db1, db2, ex_str, outer, name)\n";
	Dbptr	db1;
	Dbptr	db2;
	Dbptr	dbout;
	int	outer = 0;
	char	*ex_str = 0;
	char	*name = 0;

	if( ! PyArg_ParseTuple( args, "O&O&sO&z", parse_to_Dbptr, &db1, 
					       parse_to_Dbptr, &db2, 
					       &ex_str,
					       parse_from_Boolean, &outer, 
					       &name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	dbout = dbtheta( db1, db2, ex_str, outer, name );

	return Dbptr2PyObject( dbout );
}

static PyObject *
python_dbunjoin( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbunjoin(db, database_name, rewrite)\n";
	Dbptr	db;
	char	*database_name = 0;
	int	rewrite = 0;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&sO&", parse_to_Dbptr, &db, 
					       &database_name,
					       parse_from_Boolean, &rewrite ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbunjoin( db, database_name, rewrite );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbunjoin error" );

		return NULL;
	} 

	return Py_BuildValue( "" );
}

static PyObject *
python_dbgroup( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbgroup(db, groupfields, name, type)\n";
	Dbptr	db;
	Tbl	*groupfields = 0;
	char	*name = 0;
	long	type;

	if( ! PyArg_ParseTuple( args, "O&O&zi", parse_to_Dbptr, &db, 
					       parse_to_strtbl, &groupfields, 
					       &name,
					       &type ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dbgroup( db, groupfields, name, type );

	if( groupfields != NULL ) {

		freetbl( groupfields, 0 );
	}

	return Dbptr2PyObject( db );
}

static PyObject *
python_dbungroup( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbungroup(db, name)\n";
	PyObject *obj;
	Dbptr	db;
	char	*view_name = 0;

	if( ! PyArg_ParseTuple( args, "O&z", parse_to_Dbptr, &db, &view_name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db = dbungroup( db, view_name );

	if( db.table < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbungroup failed" );
	
		obj = NULL;

	} else {

		obj = Dbptr2PyObject( db );
	}

	return obj;
}

static PyObject *
python_db2xml( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _db2xml(db, rootnode, rownode, fields, expressions, primary)\n";
	PyObject *obj;
	Dbptr	db;
	char	*rootnode = 0;
	char	*rownode = 0; 
	char	*xml = 0;
	Tbl	*fields = 0;
	Tbl	*expressions = 0;
	int 	flags = 0;
	int	primary = 0;
	int	rc;
	
	if( ! PyArg_ParseTuple( args, "O&zzO&O&O&", parse_to_Dbptr, &db, 
						    &rootnode, &rownode, 
						    parse_to_strtbl, &fields, 
						    parse_to_strtbl, &expressions, 
						    parse_from_Boolean, &primary ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	if( primary ) {

		flags |= DBXML_PRIMARY;
	}

	rc = db2xml( db, rootnode, rownode, fields, expressions, (void **) &xml, flags );

	if( fields ) {

		freetbl( fields, 0 );
	}

	if( expressions ) {

		freetbl( expressions, 0 );
	}

	if( rc < 0 || xml == NULL) {

		PyErr_SetString( PyExc_RuntimeError, "db2xml failed" );

		return NULL;
	}

	obj = PyString_FromString( xml );

	free( xml );

	return obj;
}

static PyObject *
python_dbaddnull( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbaddnull(db)\n";
	Dbptr	db;
	long	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = dbaddnull( db );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "error adding null row" );

		return NULL;
	}

	return Py_BuildValue( "i", rc );
}

static PyObject *
python_dbaddv( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbaddv(db, field, value [, field, value...])\n";
	Dbptr	db;
	Dbvalue	value;
	char	errmsg[STRSZ];
	char	*field_name;
	int	nargs;
	int	nfields;
	int	i;
	int	type;
	int	fieldname_index;
	int	fieldval_index;
	int	retcode = 0;
	int	rc;

	nargs = PyTuple_Size( args );

	if( ( nargs < 3 ) || ( ( nargs - 1 ) % 2 != 0 ) ) {

		PyErr_SetString( PyExc_RuntimeError, usage );

		return NULL;

	} else if( ! parse_to_Dbptr( PyTuple_GetItem( args, 0 ), &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	db.record = dbNULL;

	rc = dbget( db, NULL );

	if( rc == dbINVALID ) {

		PyErr_SetString( PyExc_RuntimeError, "dbaddv: failed to get null record" );

		return NULL;
	}

	db.record = dbSCRATCH;

	nfields = ( nargs - 1 ) / 2;

	for( i = 0; i < nfields; i++ ) {

		fieldname_index = i * 2 + 1;

		if( ! PyString_Check( PyTuple_GetItem( args, fieldname_index ) ) ) {

			PyErr_SetString( PyExc_RuntimeError, usage );

			return NULL;
		}
	}

	for( i = 0; i < nfields; i++ ) {

		fieldname_index = i * 2 + 1;
		fieldval_index = fieldname_index + 1;

		field_name = PyString_AsString( PyTuple_GetItem( args, fieldname_index ) );

		db = dblookup( db, 0, 0, field_name, 0 );

		rc = dbquery( db, dbFIELD_TYPE, &type );

		if( rc == dbINVALID ) {

			sprintf( errmsg, "dbaddv: dbquery failed for field %s", field_name );

			PyErr_SetString( PyExc_RuntimeError, usage );

			return NULL;
		}

		rc = PyObject2Dbvalue( PyTuple_GetItem( args, fieldval_index ), type, &value );

		if( rc < 0 ) {
			
			sprintf( errmsg, "dbaddv: failed to convert field %s", field_name );

			PyErr_SetString( PyExc_RuntimeError, usage );

			return NULL;
		}

		switch( type ) {

		case dbDBPTR:

			retcode |= dbputv( db, 0, field_name, value.db, 0 );
			break;

		case dbSTRING:

			retcode |= dbputv( db, 0, field_name, value.t, 0 );
			break;

		case dbBOOLEAN:
		case dbINTEGER:
		case dbYEARDAY:

			retcode |= dbputv( db, 0, field_name, value.i, 0 );
			break;

		case dbREAL:
		case dbTIME:

			retcode |= dbputv( db, 0, field_name, value.d, 0 );
			break;

		default:

			retcode = -1;
			break;
		}
	}

	if( retcode != 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbaddv failed putting in one of the values\n" );

		return NULL;
	}

	retcode = dbaddchk( db, 0 );

	if( retcode == dbINVALID ) {

		PyErr_SetString( PyExc_RuntimeError, "dbaddv failed at dbaddchk call\n" );

		return NULL;
	}

	return Py_BuildValue( "i", retcode );
}

static PyObject *
python_dbgetv( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbgetv(db, field [, field...])\n";
	Dbptr	db;
	Dbvalue	val;
	PyObject *arg;
	PyObject *vals;
	char	*field;
	char	errmsg[STRSZ];
	int	type;
	int	nargs;
	int	iarg;
	int	rc;

	nargs = PyTuple_Size( args );

	if( nargs < 2 ) {

		PyErr_SetString( PyExc_RuntimeError, usage );

		return NULL;

	} else if( ! parse_to_Dbptr( PyTuple_GetItem( args, 0 ), &db ) ) {

		sprintf( errmsg, "Argument 0 to _dbgetv must be a Dbptr or four-element list of integers" );

		PyErr_SetString( PyExc_TypeError, errmsg );

		return NULL;
	}

	vals = PyTuple_New( nargs - 1 );

	for( iarg = 1; iarg < nargs; iarg++ ) {
		
		arg = PyTuple_GetItem( args, iarg );
		
		if( ! PyString_Check( arg ) ) {

			if( iarg == 1 ) {

				strcpy( errmsg, "1st " );

			} else if( iarg == 2 ) {

				strcpy( errmsg, "2nd " );

			} else if( iarg == 3 ) {

				strcpy( errmsg, "3rd " );

			} else {

				sprintf( errmsg, "%dth ", iarg );
			}

			strcat( errmsg, "field-name argument to _dbgetv must be a string" );

			PyErr_SetString( PyExc_TypeError, errmsg );

			return NULL;
		}

		field = PyString_AsString( arg );

		db = dblookup( db, 0, 0, field, 0 );

		if( db.field < 0 ) {

			sprintf( errmsg, "_dbgetv: failed to find field named '%s' in database row", field );

			PyErr_SetString( PyExc_RuntimeError, errmsg );

			return NULL;
		}

		rc = dbgetv( db, 0, field, &val, 0 );

		if( rc < 0 ) {

			sprintf( errmsg, "_dbgetv: failed to extract value for field named '%s' from database row", field );

			PyErr_SetString( PyExc_RuntimeError, errmsg );

			return NULL;
		}

		dbquery( db, dbFIELD_TYPE, &type );

		switch( type ) {

		case dbDBPTR:

			PyTuple_SetItem( vals, iarg - 1, Dbptr2PyObject( val.db ) ); 
			break;

		case dbSTRING:

			PyTuple_SetItem( vals, iarg - 1, PyString_FromString( val.s ) ); 
			break;

		case dbBOOLEAN:

			PyTuple_SetItem( vals, iarg - 1, PyBool_FromLong( val.i ) ); 
			break;

		case dbINTEGER:
		case dbYEARDAY:

			PyTuple_SetItem( vals, iarg - 1, PyInt_FromLong( val.i ) ); 
			break;

		case dbREAL:
		case dbTIME:

			PyTuple_SetItem( vals, iarg - 1, PyFloat_FromDouble( val.d ) ); 
			break;

		default:
			sprintf( errmsg, "_dbgetv internal error: type '%d' for field named '%s' not understood", type, field );

			PyErr_SetString( PyExc_RuntimeError, errmsg );

			return NULL;
		}
	}

	return vals;
}

static PyObject *
python_dbputv( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbputv(db, field, value [, field, value...])\n";
	Dbptr	db;
	Dbvalue	value;
	char	errmsg[STRSZ];
	char	*field_name;
	int	nargs;
	int	nfields;
	int	i;
	int	type;
	int	fieldname_index;
	int	fieldval_index;
	int	retcode = 0;
	int	rc;

	nargs = PyTuple_Size( args );

	if( ( nargs < 3 ) || ( ( nargs - 1 ) % 2 != 0 ) ) {

		PyErr_SetString( PyExc_RuntimeError, usage );

		return NULL;

	} else if( ! parse_to_Dbptr( PyTuple_GetItem( args, 0 ), &db ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	nfields = ( nargs - 1 ) / 2;

	for( i = 0; i < nfields; i++ ) {

		fieldname_index = i * 2 + 1;

		if( ! PyString_Check( PyTuple_GetItem( args, fieldname_index ) ) ) {

			PyErr_SetString( PyExc_RuntimeError, usage );

			return NULL;
		}
	}

	for( i = 0; i < nfields; i++ ) {

		fieldname_index = i * 2 + 1;
		fieldval_index = fieldname_index + 1;

		field_name = PyString_AsString( PyTuple_GetItem( args, fieldname_index ) );

		db = dblookup( db, 0, 0, field_name, 0 );

		rc = dbquery( db, dbFIELD_TYPE, &type );

		if( rc == dbINVALID ) {

			sprintf( errmsg, "dbputv: dbquery failed for field %s", field_name );

			PyErr_SetString( PyExc_RuntimeError, usage );

			return NULL;
		}

		rc = PyObject2Dbvalue( PyTuple_GetItem( args, fieldval_index ), type, &value );

		if( rc < 0 ) {
			
			sprintf( errmsg, "dbputv: failed to convert field %s", field_name );

			PyErr_SetString( PyExc_RuntimeError, usage );

			return NULL;
		}

		switch( type ) {

		case dbDBPTR:

			retcode |= dbputv( db, 0, field_name, value.db, 0 );
			break;

		case dbSTRING:

			retcode |= dbputv( db, 0, field_name, value.t, 0 );
			break;

		case dbBOOLEAN:
		case dbINTEGER:
		case dbYEARDAY:

			retcode |= dbputv( db, 0, field_name, value.i, 0 );
			break;

		case dbREAL:
		case dbTIME:

			retcode |= dbputv( db, 0, field_name, value.d, 0 );
			break;

		default:

			retcode = -1;
			break;
		}
	}

	if( retcode != 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "dbputv failed putting in one of the values\n" );

		return NULL;
	}

	return Py_BuildValue( "i", retcode );
}

static PyObject *
python_dbquery( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbquery(db, code)\n";
	Dbptr	db;
	PyObject *obj;
	char	*string;
	char	errmsg[STRSZ];
	Tbl	*tbl;
	Arr	*arr;
	int	dbcode;
	int	n;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&i", parse_to_Dbptr, &db, &dbcode ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	switch( dbcode )	
	{
	case dbSCHEMA_DEFAULT:
	case dbDATABASE_FILENAME:
	case dbIDSERVER:
	case dbLOCKS:
        case dbSCHEMA_DESCRIPTION:
        case dbTIMEDATE_NAME:
        case dbDATABASE_DESCRIPTION:
        case dbTABLE_DESCRIPTION:
        case dbFIELD_DESCRIPTION:
        case dbSCHEMA_DETAIL:
        case dbDATABASE_DETAIL:
        case dbTABLE_DETAIL:
        case dbFIELD_DETAIL:
        case dbSCHEMA_NAME:
        case dbDATABASE_NAME:
        case dbTABLE_NAME:
        case dbFIELD_NAME:
        case dbTABLE_FILENAME:
        case dbTABLE_DIRNAME:
        case dbFIELD_RANGE:
        case dbFIELD_FORMAT:
        case dbDBPATH:
        case dbFORMAT:
        case dbFIELD_UNITS:
        case dbFIELD_BASE_TABLE:
        case dbUNIQUE_ID_NAME:

		if( ( rc = dbquery(db, dbcode, &string) ) >= 0 ) {

			obj = Py_BuildValue( "s", string );

		} else {

			PyErr_SetString( PyExc_RuntimeError, "dbquery failed to extract value" );

			obj = NULL;
		}

		break;

        case dbDATABASE_COUNT:
        case dbTABLE_COUNT:
        case dbFIELD_COUNT:
        case dbRECORD_COUNT:
        case dbTABLE_SIZE:
        case dbFIELD_SIZE:
        case dbFIELD_INDEX:
        case dbVIEW_TABLE_COUNT:
        case dbRECORD_SIZE:
        case dbTABLE_IS_WRITEABLE:
        case dbTABLE_IS_VIEW:
	case dbDATABASE_IS_WRITABLE:
	case dbTABLE_PRESENT:
	case dbTABLE_IS_TRANSIENT:
        case dbFIELD_TYPE:

		if( ( rc = dbquery(db, dbcode, &n) ) >= 0 ) {
			
			obj = Py_BuildValue( "i", n );

		} else {

			PyErr_SetString( PyExc_RuntimeError, "dbquery failed to extract value" );

			obj = NULL;
		}

                break;  

        case dbLINK_FIELDS:

		if( ( rc = dbquery(db, dbcode, &arr) ) >= 0 ) {

			obj = strarr2PyObject( arr );

		} else {

			PyErr_SetString( PyExc_RuntimeError, "dbquery failed to extract value" );

			obj = NULL;
		}

                break;  
 
        case dbSCHEMA_FIELDS:
	case dbSCHEMA_TABLES:
        case dbFIELD_TABLES:
        case dbVIEW_TABLES:
        case dbTABLE_FIELDS:
        case dbPRIMARY_KEY:
        case dbALTERNATE_KEY:
        case dbFOREIGN_KEYS:

		if( ( rc = dbquery(db, dbcode, &tbl) ) >= 0 ) {

			obj = strtbl2PyObject( tbl );

		} else {

			PyErr_SetString( PyExc_RuntimeError, "dbquery failed to extract value" );

			obj = NULL;
		}

                break;  
 
        default:

		sprintf( errmsg, "dbquery: bad code '%d'", dbcode );

		PyErr_SetString( PyExc_RuntimeError, errmsg );

		obj = NULL;

		break ;
	}

	return obj;
}

static PyObject *
python_dbnextid( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _dbnextid(db, name)\n";
	Dbptr	db;
	long	id;
	char	*name;

	if( ! PyArg_ParseTuple( args, "O&s", parse_to_Dbptr, &db, &name ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	id = dbnextid( db, name );

	if( id == -1 ) {

		PyErr_SetString( PyExc_RuntimeError, "unable to update lastid table" );

		return NULL;
	}

	return Py_BuildValue( "i", id );
}

static PyObject *
python_trfilter( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trfilter(tr, filter_string)";
	Dbptr	tr;
	char	*filter_string;
	int	rc;
	
	if( ! PyArg_ParseTuple( args, "O&s", parse_to_Dbptr, &tr, &filter_string ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trfilter( tr, filter_string );

	return Py_BuildValue( "i", rc );
}

static PyObject *
python_trsample( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trsample(db, t0, t1, sta, chan, apply_calib)";
	Dbptr	db;
	Dbptr	tr;
	double	t0;
	double	t1;
	double	time;
	double	row_starttime;
	double	samprate;
	float	*data;
	char	*sta;
	char	*chan;
	int	apply_calib = 0;
	int	nrows = 0;
	int	nsamp_total = 0;
	int	nsamp_row = 0;
	int	irow;
	int	isamp_row = 0;
	int	isamp_total = 0;
	Tbl	*s;
	PyObject *obj;
	PyObject *pair;

	if( ! PyArg_ParseTuple( args, "O&ddssO&", parse_to_Dbptr, 
				       &db, &t0, &t1, &sta, &chan, parse_from_Boolean, &apply_calib ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	tr = trloadchan( db, t0, t1, sta, chan );

	tr = dbsort( tr, s = strtbl( "time", 0 ), 0, 0 );

	freetbl( s, 0 );

	trsplice( tr, trTOLERANCE, 0, 0 );

	if( apply_calib ) {

		trapply_calib( tr );
	}

	dbquery( tr, dbRECORD_COUNT, &nrows );

	if( nrows <= 0 ) {

		PyErr_SetString( PyExc_RuntimeError, 
			"trsample: no data (no rows in trace object)\n" );

		return NULL;
	}

	for( irow = 0; irow < nrows; irow++ ) {

		tr.record = irow;

		dbgetv( tr, 0, "nsamp", &nsamp_row, 0 );

		nsamp_total += nsamp_row;
	}

	if( nsamp_total <= 0 ) {

		PyErr_SetString( PyExc_RuntimeError, 
			"trsample: no data (no samples in trace object rows)\n" );

		return NULL;
	}

	obj = PyTuple_New( nsamp_total );

	for( irow = 0; irow < nrows; irow++ ) {

		tr.record = irow;

		dbgetv( tr, 0, "nsamp", &nsamp_row, 
			       "time", &row_starttime, 
			       "samprate", &samprate, 
			       "data", &data, 0 );
		
		for( isamp_row = 0; isamp_row < nsamp_row; isamp_row++ ) {

			time = SAMP2TIME( row_starttime, samprate, isamp_row );

			pair = PyTuple_New( 2 );

			PyTuple_SetItem( pair, 0, PyFloat_FromDouble( time ) );
			PyTuple_SetItem( pair, 1, PyFloat_FromDouble( (double) data[isamp_row] ) );

			PyTuple_SetItem( obj, isamp_total, pair );

			isamp_total++;
		}
	}

	return obj;
}

static PyObject *
python_trloadchan( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trloadchan(db, t0, t1, sta, chan)\n";
	Dbptr	db;
	Dbptr	tr;
	double	t0;
	double	t1;
	char	*sta;
	char	*chan;

	if( ! PyArg_ParseTuple( args, "O&ddss", parse_to_Dbptr, 
				       &db, &t0, &t1, &sta, &chan ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	tr = trloadchan( db, t0, t1, sta, chan ); 

	return Dbptr2PyObject( tr );
}

static PyObject *
python_trdata( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trdata(tr)\n";
	Dbptr	tr;
	float	*data;
	int	result;
	int	nsamp;
	int	i;
	PyObject *obj;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &tr ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	if( tr.record < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, 
			"trdata requires a single record number in tr.record\n" );

		return NULL;
	} 

	result = dbgetv( tr, 0, "nsamp", &nsamp, "data", &data, 0 );

	if( result != 0 || data == 0 ) {

		PyErr_SetString( PyExc_RuntimeError, 
			"trdata failed to retrieve nsamp and data-pointer from record\n" );

		return NULL;
	}

	obj = PyTuple_New( nsamp );

	for( i = 0; i < nsamp; i++ ) {

		PyTuple_SetItem( obj, i, PyFloat_FromDouble( data[i] ) );
	}

	return obj;
}

static PyObject *
python_trsplice( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trsplice(tr)\n";
	Dbptr	tr;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &tr ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trsplice( tr, trTOLERANCE, 0, 0 );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "splice error" );

		return NULL;
	} 

	return Py_BuildValue( "" );
}

static PyObject *
python_trlookup_segtype( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trlookup_segtype(segtype)\n";
	char	*segtype;
	char	*segunits = 0;
	char	*segdesc = 0;
	PyObject *obj;
	int	rc;

	if( ! PyArg_ParseTuple( args, "s", &segtype ) ) {

		PyErr_SetString( PyExc_RuntimeError, usage );

		return NULL;
	}

	rc = trlookup_segtype( segtype, &segunits, &segdesc );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "trlookup_segtype error" );

		return NULL;
	}

	obj = PyTuple_New( 2 );

	PyTuple_SetItem( obj, 0, PyString_FromString( segunits ) );
	PyTuple_SetItem( obj, 1, PyString_FromString( segdesc ) );

	return obj;
}

static PyObject *
python_trwfname( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trwfname(db, pattern)\n";
	Dbptr	db;
	int	rc;
	char	*pattern;
	char	*path = 0;
	PyObject *obj;

	if( ! PyArg_ParseTuple( args, "O&s", parse_to_Dbptr, &db, &pattern ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trwfname( db, pattern, &path );

	if( rc == -1 ) {

		PyErr_SetString( PyExc_RuntimeError, "trwfname error (problem with path)" );

		return NULL;

	} else if( rc == -2 ) {

		PyErr_SetString( PyExc_RuntimeError, "trwfname error (dir or dfile too large for database fields)" );

		return NULL;

	} else if( path == NULL ) {

		PyErr_SetString( PyExc_RuntimeError, "trwfname error (empty path)" );

		return NULL;
	}

	obj = Py_BuildValue( "s", path );

	free( path );

	return obj;
}

static PyObject *
python_trtruncate( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trtruncate(tr, leave)\n";
	Dbptr	tr;
	int	rc;
	int	leave;

	if( ! PyArg_ParseTuple( args, "O&i", parse_to_Dbptr, &tr, &leave ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trtruncate( tr, leave );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "trtruncate error" );

		return NULL;
	} 

	return Py_BuildValue( "" );
}

static PyObject *
python_trfree( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trfree(tr)\n";
	Dbptr	tr;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &tr ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trfree( tr );

	return Py_BuildValue( "i", rc );
}

static PyObject *
python_trdestroy( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trdestroy(tr)\n";
	Dbptr	tr;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &tr ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trdestroy( &tr );

	return Py_BuildValue( "i", rc );
}

static PyObject *
python_trsplit( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trsplit(tr)\n";
	Dbptr	tr;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&", parse_to_Dbptr, &tr ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trsplit( tr, 0, 0 );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "split error" );

		return NULL;
	} 

	return Py_BuildValue( "" );
}

static PyObject *
python_trcopy( PyObject *self, PyObject *args ) {
	char	*usage = "Usage: _trcopy(trout, trin)\n";
	Dbptr	trin;
	Dbptr	trout;
	int	rc;

	if( ! PyArg_ParseTuple( args, "O&O&", parse_to_Dbptr, &trout, parse_to_Dbptr, &trin ) ) {

		if( ! PyErr_Occurred() ) {

			PyErr_SetString( PyExc_RuntimeError, usage );
		}

		return NULL;
	}

	rc = trcopy( &trout, trin );

	if( rc < 0 ) {

		PyErr_SetString( PyExc_RuntimeError, "trcopy: unknown error\n" );

		return NULL;

	} else {

		return Dbptr2PyObject( trout );
	}
}

static void
add_datascope_constants( PyObject *mod ) {
	int	i;

	for( i = 0; i < NDbxlat; i++ ) {

		PyModule_AddIntConstant( mod, Dbxlat[i].name, Dbxlat[i].num );
	}
	
	return;	
}

PyMODINIT_FUNC
init_datascope( void ) {
	PyObject *mod;

	mod = Py_InitModule( "_datascope", _datascope_methods );

	add_datascope_constants( mod );
}
