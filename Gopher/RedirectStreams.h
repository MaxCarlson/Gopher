#pragma once
#include <stdio.h>
#include <io.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#pragma warning( push )
#pragma warning(disable : 4996)

// Scoped redirect of a standard stream 
// StdFile = the stream code for the std stream
// you wish to redirect
struct RedirectStream
{
	RedirectStream(int StdFile = STDOUT_FILENO) : FileCode(StdFile)
	{
		/* duplicate std(x) */
		std_dupfd = _dup(FileCode);

		temp_out = fopen("file.txt", "w");

		/* replace stdout with our output fd */
		_dup2(_fileno(temp_out), FileCode);

		/* output something... */
		//printf("Woot!\n");
		/* flush output so it goes to our file */
		fflush(__acrt_iob_func(FileCode));
	}
	~RedirectStream()
	{
		fclose(temp_out);
		/* Now restore stdout */
		_dup2(std_dupfd, FileCode);
		_close(std_dupfd);
	}
	int FileCode;
	int std_dupfd;
	FILE *temp_out;
};
#pragma warning( pop )