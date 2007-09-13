#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <glib/gstdio.h>

#include "gvfsdaemonbackendtest.h"
#include "gvfsjobopenforread.h"

G_DEFINE_TYPE (GVfsDaemonBackendTest, g_vfs_daemon_backend_test, G_TYPE_VFS_DAEMON_BACKEND);

static gboolean open_for_read (GVfsDaemonBackend *backend,
			       GVfsJobOpenForRead *job,
			       char *filename);

static void
g_vfs_daemon_backend_test_finalize (GObject *object)
{
  GVfsDaemonBackendTest *daemon;

  daemon = G_VFS_DAEMON_BACKEND_TEST (object);
  
  if (G_OBJECT_CLASS (g_vfs_daemon_backend_test_parent_class)->finalize)
    (*G_OBJECT_CLASS (g_vfs_daemon_backend_test_parent_class)->finalize) (object);
}

static void
g_vfs_daemon_backend_test_class_init (GVfsDaemonBackendTestClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GVfsDaemonBackendClass *backend_class = G_VFS_DAEMON_BACKEND_CLASS (klass);
  
  gobject_class->finalize = g_vfs_daemon_backend_test_finalize;

  backend_class->open_for_read = open_for_read;
}

static void
g_vfs_daemon_backend_test_init (GVfsDaemonBackendTest *daemon)
{
}

GVfsDaemonBackendTest *
g_vfs_daemon_backend_test_new (void)
{
  GVfsDaemonBackendTest *backend;
  backend = g_object_new (G_TYPE_VFS_DAEMON_BACKEND_TEST,
			 NULL);
  return backend;
}

static gboolean 
open_idle_cb (gpointer data)
{
  GVfsJobOpenForRead *job = data;
  int fd;
  
  fd = g_open (job->filename, O_RDONLY);
  if (fd == -1)
    {
      g_vfs_job_failed (G_VFS_JOB (job), G_FILE_ERROR,
			g_file_error_from_errno (errno),
			"Error opening file %s: %s",
			job->filename, g_strerror (errno));
    }
  else
    {
      g_vfs_job_open_for_read_set_handle (job, GINT_TO_POINTER (fd));
      g_vfs_job_succeeded (G_VFS_JOB (job));
    }
  return FALSE;
}

static gboolean 
open_for_read (GVfsDaemonBackend *backend,
	       GVfsJobOpenForRead *job,
	       char *filename)
{
  GError *error;

  if (strcmp (filename, "/fail") == 0)
    {
      error = g_error_new (G_FILE_ERROR, G_FILE_ERROR_IO, "Test error");
      g_vfs_job_failed_from_error (G_VFS_JOB (job), error);
      return TRUE;
    }
  else
    {
      g_idle_add (open_idle_cb, job);
      return TRUE;
    }
}
