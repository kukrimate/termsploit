#define PY_SSIZE_T_CLEAN
#define const
#include <Python.h>
#include <termsploit.h>

typedef struct {
	PyObject_HEAD
	termsploit_ctx *ctx;
} pysploit_object;

static int
pysploit_init(pysploit_object *self, PyObject *args, PyObject *kwds)
{
	/* NOTE: no error checking here,
		failure on malloc aborts inside the library */
	self->ctx = termsploit_alloc();
	return 0;
}

static void
pysploit_dealloc(pysploit_object *self)
{
	termsploit_free(self->ctx);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
pysploit_spawn(pysploit_object *self, PyObject *arg)
{
	char **args;
	size_t i;

	args = calloc(PyList_Size(arg) + 1, sizeof(char *));
	if (!args)
		abort();

	for (i = 0; i < PyList_Size(arg); ++i) {
		args[i] = PyUnicode_AsUTF8(PyList_GetItem(arg, i));
	}
	args[i] = NULL;

	if (-1 == termsploit_spawn(self->ctx, args)) {
		free(args);
		PyErr_SetString(PyExc_OSError, strerror(errno));
		return NULL;
	}

	free(args);
	Py_RETURN_NONE;
}

static PyObject *
pysploit_connect(pysploit_object *self, PyObject *args)
{
	char *host;
	uint16_t port;

	PyArg_ParseTuple(args, "sH", &host, &port);
	if (-1 == termsploit_connect(self->ctx, host, port)) {
		PyErr_SetString(PyExc_OSError, strerror(errno));
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject *
pysploit_read(pysploit_object *self, PyObject *arg)
{
	size_t l;
	char buf[PyLong_AsSize_t(arg)];

	l = termsploit_read(self->ctx, buf, sizeof(buf));
	if (-1 == l) {
		PyErr_SetString(PyExc_OSError, strerror(errno));
		return NULL;
	}
	return PyBytes_FromStringAndSize(buf, l);
}

static PyObject *
pysploit_getline(pysploit_object *self, PyObject *Py_UNUSED(ignored))
{
	char *line;
	PyObject *bytes;

	line = termsploit_getline(self->ctx);
	bytes = PyBytes_FromStringAndSize(line, strlen(line));
	free(line);
	return bytes;
}


static PyObject *
pysploit_write(pysploit_object *self, PyObject *arg)
{
	if (-1 == termsploit_write(self->ctx,
			PyBytes_AsString(arg), PyBytes_Size(arg))) {
		PyErr_SetString(PyExc_OSError, strerror(errno));
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject *
pysploit_interactive(pysploit_object *self, PyObject *Py_UNUSED(ignored))
{
	termsploit_interactive(self->ctx);
	Py_RETURN_NONE;
}

static PyObject *
pysploit_kill(pysploit_object *self, PyObject *arg)
{
	termsploit_kill(self->ctx, PyLong_AsLong(arg));
	Py_RETURN_NONE;
}

static PyObject *
pysploit_wait(pysploit_object *self, PyObject *Py_UNUSED(ignored))
{
	int exitcode;

	exitcode = termsploit_wait(self->ctx);
	if (-1 == exitcode) {
		PyErr_SetString(PyExc_OSError, strerror(errno));
		return NULL;
	}
	return Py_BuildValue("i", exitcode);
}

static PyMethodDef pysploit_methods[] = {
	{ "spawn",
		(PyCFunction) pysploit_spawn, METH_O },
	{ "connect",
		(PyCFunction) pysploit_connect, METH_VARARGS },
	{ "read",
		(PyCFunction) pysploit_read, METH_O },
	{ "getline",
		(PyCFunction) pysploit_getline, METH_NOARGS },
	{ "write",
		(PyCFunction) pysploit_write, METH_O},
	{ "interactive",
		(PyCFunction) pysploit_interactive, METH_NOARGS },
	{ "kill",
		(PyCFunction) pysploit_kill, METH_O },
	{ "wait",
		(PyCFunction) pysploit_wait, METH_NOARGS },
	{ NULL }  /* Sentinel */
};

static PyTypeObject pysploit_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name      = "pysploit.ctx",
	.tp_basicsize = sizeof(pysploit_object),
	.tp_itemsize  = 0,
	.tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_new       = PyType_GenericNew,
	.tp_init      = (initproc) pysploit_init,
	.tp_dealloc   = (destructor) pysploit_dealloc,
	.tp_methods   = pysploit_methods,
};

static PyModuleDef pysploit_module = {
	PyModuleDef_HEAD_INIT,
	.m_name = "pysploit",
	.m_size = -1,
};

PyMODINIT_FUNC
PyInit_pysploit(void)
{
	PyObject *m;
	if (PyType_Ready(&pysploit_type) < 0)
		return NULL;

	m = PyModule_Create(&pysploit_module);
	if (m == NULL)
		return NULL;

	Py_INCREF(&pysploit_type);
	if (PyModule_AddObject(m, "ctx", (PyObject *) &pysploit_type) < 0) {
		Py_DECREF(&pysploit_type);
		Py_DECREF(m);
		return NULL;
	}

	return m;
}
