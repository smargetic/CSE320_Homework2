#include <sys/file.h>
#include <stdio.h>
#include <ctype.h>
#include <sgtty.h>
#include <signal.h>
#include <pwd.h>
//cdir(home/student/smargetic/hw2/include)
//These are the libraries that I included
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "sys5.h"
//#include "include/sys5.h"

#ifdef TMC
#include <ctools.h>
#else
#include "ctools.h"
#endif
#include "args.h"
#include "menu.h"
#include "mem.h"

#include "rolofiles.h"
#include "rolodefs.h"
#include "datadef.h"
//debug ("\nWE GOT HERE")

//I wrote this
#include "io.h"
#include "clear.h"
#include "rlist.h"
#include "search.h"
#include "update.h"
#include "options.h"
#include "menuaux.h"

static char rolodir[DIRPATHLEN];        /* directory where rolo data is */
static char filebuf[DIRPATHLEN];        /* stores result of homedir() */

int changed = 0;
int reorder_file = 0;
int rololocked = 0;
int in_search_mode = 0;

void save_to_disk();
void save_and_exit(int rval);
void roloexit(int rval);
Bool rolo_only_to_read(); //changed this to Bool
void user_eof();
void locked_action();

void save_and_exit (int rval)
{
  if (changed) save_to_disk();
  roloexit(rval);
}


char *rolo_emalloc (int size)

/* error handling memory allocator */

{
  char *rval;
  rval = malloc(size);
  if (0 == rval) {
     fprintf(stderr,"Fatal error:  out of memory\n");
     save_and_exit(-1);
  }
  return(rval);
}


char *copystr (char *s)

/* memory allocating string copy routine */


{
 char *copy;
 if (s == 0) return(0);
 copy = rolo_emalloc(strlen(s)+1);
 strcpy(copy,s);
 return(copy);
}


char *timestring ()

/* returns a string timestamp */

{
  char *s;
  long timeval;
  time(&timeval);
  s = ctime(&timeval);
  s[strlen(s) - 1] = '\0';
  return(copystr(s));
}


void user_interrupt ()

/* if the user hits C-C (we assume he does it deliberately) */

{
  unlink(homedir(ROLOLOCK));
  fprintf(stderr,"\nAborting rolodex, no changes since last save recorded\n");
  exit(0);
  //roloexit(0);
}


void user_eof ()

/* if the user hits C-D */

{
  unlink(homedir(ROLOLOCK));
  fprintf(stderr,"\nUnexpected EOF on terminal. Saving rolodex and exiting\n");
  save_and_exit(-1);
}


void roloexit (int rval)
{
  if (rololocked) unlink(homedir(ROLOLOCK));
  //printf("\nVALUE OF ROLOEXIT: %d", rval);
  exit(rval);
}


void save_to_disk ()

/* move the old rolodex to a backup, and write out the new rolodex and */
/* a copy of the new rolodex (just for safety) */

{
  FILE *tempfp,*copyfp;
  char d1[DIRPATHLEN], d2[DIRPATHLEN];
  //int r;

  tempfp = fopen(homedir(ROLOTEMP),"w");
  copyfp = fopen(homedir(ROLOCOPY),"w");
  if (tempfp == NULL || copyfp == NULL) {
     fprintf(stderr,"Unable to write rolodex...\n");
     fprintf(stderr,"Any changes made have not been recorded\n");
     roloexit(-1);
  }
  write_rolo(tempfp,copyfp);
  if((fclose(tempfp) != 0 )|| (fclose(copyfp) != 0)){
    roloexit(-1);
  }
  if (rename(strcpy(d1,homedir(ROLODATA)),strcpy(d2,homedir(ROLOBAK))) ||
      rename(strcpy(d1,homedir(ROLOTEMP)),strcpy(d2,homedir(ROLODATA)))) {
     fprintf(stderr,"Rename failed.  Revised rolodex is in %s\n",ROLOCOPY);
     roloexit(-1);
  }
  printf("Rolodex saved\n");
  sleep(1);
  changed = 0;
}


extern struct passwd *getpwnam();

char *home_directory (char* name)
{
  struct passwd *pwentry;
  if (0 == (pwentry = getpwnam(name))) return("");
  return(pwentry -> pw_dir);
}


char *homedir (char* filename)

/* e.g., given "rolodex.dat", create "/u/massar/rolodex.dat" */
/* rolodir generally the user's home directory but could be someone else's */
/* home directory if the -u option is used. */

{
  nbuffconcat(filebuf,3,rolodir,"/",filename);
  return(filebuf);
}


char *libdir (char* filename)

/* return a full pathname into the rolodex library directory */
/* the string must be copied if it is to be saved! */

{
  nbuffconcat(filebuf,3,ROLOLIB,"/",filename);
  return(filebuf);
}


// Bool rolo_only_to_read () //was bool before
// {
//   return(option_present(SUMMARYFLAG) || n_non_option_args() > 0);
// }


void locked_action (int option)
{
  if (option == 1) {
     fprintf(stderr,"Someone else is modifying that rolodex, sorry\n");
     exit(-1);
     // roloexit(-1);
  }
  else {
     cathelpfile(libdir("lockinfo"),"locked rolodex",0);
      // roloexit(-1);
     exit(-1);
  }
}

void rolo_main(int argc, char* argv[]){
    int fd,in_use,read_only,rolofd;
    //Bool not_own_rolodex;
    char *user;
    FILE *tempfp;
    //FILE *rolofd;

    clearinit();
    clear_the_screen();
    int num_non_opt = 0;
    int option = 0;
    int x;

    //printf("----------------0-1\n");
    while((x = getopt(argc, argv, "lsu:")) != -1){
      switch(x){
        case 'u':
          option = 1;
          user = optarg;

          strcpy(rolodir,home_directory(user));
            if (*rolodir == '\0') {

               fprintf(stderr,"No user %s is known to the system\n",user);
               roloexit(-1);
            }
          break;
        case 's':
          option = 2;
          break;
        case 'l':
          option = 3;
          break;
        default:
          roloexit(-1);
      }
    }

    //printf("----------------0\n");
    // if(option==1){
    //              if (*rolodir == '\0') {

    //           fprintf(stderr,"No user %s is known to the system\n",user);
    //           roloexit(-1);
    //        }
    // }
    //int truth = optind;
    char ** extraStuff = malloc(52*sizeof(char*));
    int add = 0;
    int truth = optind;
    while(truth <argc){
      int len = strlen(argv[truth]);
      //int len2 = strlen
      char* temp = malloc(len*sizeof(char));
      temp = argv[truth];
      //extraStuff = extraStuff+ add;
      *(extraStuff+ add) = malloc(len*sizeof(char));
      strcpy(*(extraStuff+add),temp);
      // free(temp);
      truth++;
      add++;
      num_non_opt++;
    }

    //printf("----------------------------1\n");

     /* find the directory in which the rolodex file we want to use is */

    //not_own_rolodex = option_present(OTHERUSERFLAG);
    // if (not_own_rolodex) {
    //    // if (NIL == (user = option_arg(OTHERUSERFLAG,1)) ||
    //    //     n_option_args(OTHERUSERFLAG) != 1) {
    //    //    fprintf(stderr,"Illegal syntax using -u option\nusage: %s\n",USAGE);
    //    //    roloexit(-1);
    //    // }
    // }
    if(option != 1) {
       if (0 == (user = getenv("HOME"))) {
          fprintf(stderr,"Cant find your home directory, no HOME\n");
          roloexit(-1);
       }
       strcpy(rolodir,user);
    }

    // if (not_own_rolodex) {

    // }
    // else
    /* is the rolodex readable? */
    //printf("----------------------------2\n");

    if (0 != access(homedir(ROLODATA),R_OK)) {

       /* No.  if it exists and we cant read it, that's an error */

       if (0 == access(homedir(ROLODATA),F_OK)) {
          fprintf(stderr,"Cant access rolodex data file to read\n");
          roloexit(-1);
       }

       /* if it doesn't exist, should we create one? */

       if (option == 1) {
          fprintf(stderr,"No rolodex file belonging to %s found\n",user);
          roloexit(-1);
       }

       /* try to create it */

       if (-1 == (fd = creat(homedir(ROLODATA),0644))) {
          fprintf(stderr,"couldnt create rolodex in your home directory\n");
          roloexit(-1);
       }

       else {
          close(fd);
          fprintf(stderr,"Creating empty rolodex...\n");
       }

    }

    //printf("----------------------------3\n");
    /* see if someone else is using it */

    in_use = (0 == access(homedir(ROLOLOCK),F_OK));

    /* are we going to access the rolodex only for reading? */

    if (!(read_only =  ((option == 2) || (num_non_opt > 0)))) {

       /* No.  Make sure no one else has it locked. */

       if (in_use) {
          locked_action(option);
       }

       /* create a lock file.  Catch interrupts so that we can remove */
       /* the lock file if the user decides to abort */

       if (option != 3) {
          if ((fd = open(homedir(ROLOLOCK),O_EXCL|O_CREAT,00200|00400)) < 0) {
             fprintf(stderr,"unable to create lock file...\n");
       exit(1);
       //roloexit(-1);
    }
          rololocked = 1;
          close(fd);
          signal(SIGINT,user_interrupt);
       }

       /* open a temporary file for writing changes to make sure we can */
       /* write into the directory */

       /* when the rolodex is saved, the old rolodex is moved to */
       /* a '~' file, the temporary is made to be the new rolodex, */
       /* and a copy of the new rolodex is made */

       tempfp = fopen(homedir(ROLOTEMP),"w");
       if (tempfp == NULL) {
           fprintf(stderr,"Can't open temporary file to write to\n");
           roloexit(-1);
       }
       fclose(tempfp);

    }

    //printf("----------------------------4\n");
    allocate_memory_chunk(CHUNKSIZE);
    rolofd = open(homedir(ROLODATA),O_RDONLY);
    if (rolofd == -1) { //I CHANGED THIS TO '\0'
        fprintf(stderr,"Can't open rolodex data file to read\n");
        roloexit(-1);
    }

    /* read in the rolodex from disk */
    /* It should never be out of order since it is written to disk ordered */
    /* but just in case... */

    if (!read_only) printf("Reading in rolodex from %s\n",homedir(ROLODATA));
    read_rolodex(rolofd);
    close(rolofd);
    if (!read_only) printf("%d entries listed\n",rlength(Begin_Rlist));
    if (reorder_file && !read_only) {
       fprintf(stderr,"Reordering rolodex...\n");
       rolo_reorder();
       fprintf(stderr,"Saving reordered rolodex to disk...\n");
       save_to_disk();
    }

    /* the following routines live in 'options.c' */

    /* -s option.  Prints a short listing of people and phone numbers to */
    /* standard output */

    if (option == 2) {
        print_short();
        exit(0);
        //roloexit(0);
    }

    /* rolo <name1> <name2> ... */
    /* print out info about people whose names contain any of the arguments */

    if (num_non_opt > 0) {
       print_people(extraStuff);
       exit(0);
       //roloexit(0);
    }
    //printf("----------------------------5\n");
    /* regular rolodex program */
    //putchar(0x01);
    interactive_rolo();
//    putchar(0x0);
    //printf("----------------------------6\n");
    exit(0);
    //return(0);
    //roloexit(0);

}

// void rolo_main (int argc, char* argv[])

// {
//     int fd,in_use,read_only,rolofd;
//     Bool not_own_rolodex;
//     char *user;
//     FILE *tempfp;
//     //FILE *rolofd;

//     clearinit();
//     clear_the_screen();

//     /* parse the options and arguments, if any */



//     switch (get_args(argc,argv,T,T)) {
//         case ARG_ERROR :
//           roloexit(-1);
//         case NO_ARGS :
//           break;
//         case ARGS_PRESENT :
//           if (ALL_LEGAL != legal_options(LEGAL_OPTIONS)) {
//                 fprintf(stderr,"illegal option\nusage: %s\n",USAGE);
//                 roloexit(-1);
//           }
//     }

//     /* find the directory in which the rolodex file we want to use is */

//     not_own_rolodex = option_present(OTHERUSERFLAG);
//     if (not_own_rolodex) {
//        if (NIL == (user = option_arg(OTHERUSERFLAG,1)) ||
//            n_option_args(OTHERUSERFLAG) != 1) {
//           fprintf(stderr,"Illegal syntax using -u option\nusage: %s\n",USAGE);
//           roloexit(-1);
//        }
//     }
//     else {
//        if (0 == (user = getenv("HOME"))) {
//           fprintf(stderr,"Cant find your home directory, no HOME\n");
//           roloexit(-1);
//        }
//     }

//     if (not_own_rolodex) {
//        strcpy(rolodir,home_directory(user));
//        if (*rolodir == '\0') {
//           fprintf(stderr,"No user %s is known to the system\n",user);
//           roloexit(-1);
//        }
//     }
//     else strcpy(rolodir,user);

//     /* is the rolodex readable? */

//     if (0 != access(homedir(ROLODATA),R_OK)) {

//        /* No.  if it exists and we cant read it, that's an error */

//        if (0 == access(homedir(ROLODATA),F_OK)) {
//           fprintf(stderr,"Cant access rolodex data file to read\n");
//           roloexit(-1);
//        }

//        /* if it doesn't exist, should we create one? */

//        if (option_present(OTHERUSERFLAG)) {
//           fprintf(stderr,"No rolodex file belonging to %s found\n",user);
//           roloexit(-1);
//        }

//        /* try to create it */

//        if (-1 == (fd = creat(homedir(ROLODATA),0644))) {
//           fprintf(stderr,"couldnt create rolodex in your home directory\n");
//           roloexit(-1);
//        }

//        else {
//           close(fd);
//           fprintf(stderr,"Creating empty rolodex...\n");
//        }

//     }

//     /* see if someone else is using it */

//     in_use = (0 == access(homedir(ROLOLOCK),F_OK));

//     /* are we going to access the rolodex only for reading? */

//     if (!(read_only = rolo_only_to_read())) {

//         No.  Make sure no one else has it locked.

//        if (in_use) {
//           locked_action();
//        }

//        /* create a lock file.  Catch interrupts so that we can remove */
//        /* the lock file if the user decides to abort */

//        if (!option_present(NOLOCKFLAG)) {
//           if ((fd = open(homedir(ROLOLOCK),O_EXCL|O_CREAT,00200|00400)) < 0) {
//              fprintf(stderr,"unable to create lock file...\n");
// 	     exit(1);
// 	     //roloexit(-1);
//     }
//           rololocked = 1;
//           close(fd);
//           signal(SIGINT,user_interrupt);
//        }

//        /* open a temporary file for writing changes to make sure we can */
//        /* write into the directory */

//        /* when the rolodex is saved, the old rolodex is moved to */
//        /* a '~' file, the temporary is made to be the new rolodex, */
//        /* and a copy of the new rolodex is made */

//        tempfp = fopen(homedir(ROLOTEMP),"w");
//        if (tempfp == NULL) {
//            fprintf(stderr,"Can't open temporary file to write to\n");
//            roloexit(-1);
//        }
//        fclose(tempfp);

//     }

//     allocate_memory_chunk(CHUNKSIZE);
//     rolofd = open(homedir(ROLODATA),O_RDONLY);
//     if (rolofd == '\0') { //I CHANGED THIS TO '\0'
//         fprintf(stderr,"Can't open rolodex data file to read\n");
//         roloexit(-1);
//     }

//     /* read in the rolodex from disk */
//     /* It should never be out of order since it is written to disk ordered */
//     /* but just in case... */

//     if (!read_only) printf("Reading in rolodex from %s\n",homedir(ROLODATA));
//     read_rolodex(rolofd);
//     close(rolofd);
//     if (!read_only) printf("%d entries listed\n",rlength(Begin_Rlist));
//     if (reorder_file && !read_only) {
//        fprintf(stderr,"Reordering rolodex...\n");
//        rolo_reorder();
//        fprintf(stderr,"Saving reordered rolodex to disk...\n");
//        save_to_disk();
//     }

//     /* the following routines live in 'options.c' */

//     /* -s option.  Prints a short listing of people and phone numbers to */
//     /* standard output */

//     if (option_present(SUMMARYFLAG)) {
//         print_short();
//         //exit(0);
//         roloexit(0);
//     }

//     /* rolo <name1> <name2> ... */
//     /* print out info about people whose names contain any of the arguments */

//     if (n_non_option_args() > 0) {
//        print_people();
//        //exit(0);
//        roloexit(0);
//     }

//     /* regular rolodex program */

//     interactive_rolo();
//     //exit(0);
//     roloexit(0);
// }
