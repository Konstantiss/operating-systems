void runcmd(struct cmd *cmd)
{
  int p[2], r;
  struct execcmd *execcmd;
  struct pipecmd *pipecmd;
  struct redircmd *redircmd;

  if(cmd == 0)
    _exit(0);

  switch(cmd->type){
  default:
    fprintf(stderr, "unknown run command\n");
    _exit(-1);

  case ' ':
    execcmd = (struct execcmd*)cmd;
    if(execcmd->argv[0] == 0)
    {
      _exit(-1);
    }
    execvp(execcmd->argv[0], execcmd->argv);
    break;

  case '>':
  case '<':
    redircmd = (struct redircmd*)cmd;
    close(redircmd->fd);
    if(open(redircmd->file, redircmd->flags, 00700) < 0)
    {
      fprintf(stderr,"Error, could not open file: %s .",redircmd->file);  
      _exit(-1);
    }
    runcmd(redircmd->cmd);
    close(redircmd->fd);
    break;

  case '|':
    pipecmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
    {
      fprintf(stderr,"Error, could not create pipeline.");
      _exit(-1);
    }
    if(fork() == 0){
      close(1);
      if(dup(p[1])<0)
      {
        fprintf(stderr,"Error, could not dup.");
        _exit(-1);
      }
      close(p[0]);
      close(p[1]);
      runcmd(pipecmd->left);
    }
    if(fork() == 0)
    {
      close(0);
      if(dup(p[0])<0)
      {
        fprintf(stderr,"Error, could not dup.");
        _exit(-1);
      }
      close(p[0]);
      close(p[1]);
      runcmd(pipecmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait(&r);
    wait(&r);
    break;
  }
  _exit(0);
}