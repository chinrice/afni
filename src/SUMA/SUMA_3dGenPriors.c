#define MAIN

#include "SUMA_suma.h"
#include "thd_segtools_fNM.h"
#include "SUMA_SegOpts.h"
#include "SUMA_SegFunc.h"
#include "matrix.h"

static int vn=0 ;

static void vstep_print(void)
{
   static char xx[10] = "0123456789" ;
   fprintf(stderr , "%c" , xx[vn%10] ) ;
   if( vn%10 == 9) fprintf(stderr,".") ;
   vn++ ;
}


int GenPriors(SEG_OPTS *Opt) 
{
   
   ENTRY("GenPriors");
   
   
   /* get the probability maps */
   if (!Opt->pset && Opt->DO_p) {
      if (!(Opt->pset = p_C_GIV_A(Opt))) {
         ERROR_message("Failed miserably");
         RETURN(0);
      }
   }
      
   /* Get the classes */
   if (!Opt->cset && Opt->crefix && Opt->DO_c) {
      if (!(SUMA_assign_classes_eng(Opt->pset, 
                           Opt->clss->str, Opt->clss->num, Opt->keys, 
                           Opt->cmask, &Opt->cset))) {
         ERROR_message("Failed aimlessly");
         RETURN(0);
      }
      EDIT_dset_items(Opt->cset, ADN_prefix, Opt->crefix, ADN_none);
      if( !THD_ok_overwrite() && THD_is_file( DSET_HEADNAME(Opt->cset) ) ){
      ERROR_exit("Output file %s already exists -- cannot continue!\n",
                  DSET_HEADNAME(Opt->cset) ) ;
      }
   }  
   
   /* group classes ? */
   if (Opt->group_classes) {
      THD_3dim_dataset *gcset=NULL;
      THD_3dim_dataset *gpset=NULL;
      if (!SUMA_Regroup_classes (Opt, 
                     Opt->clss->str, Opt->clss->num, Opt->keys,
                     Opt->group_classes->str,
                     Opt->group_classes->num,
                     Opt->group_keys, Opt->cmask,
                     Opt->pset, 
                     Opt->cset,
                     &gpset, 
                     &gcset) ) {
         ERROR_message("Failed to regroup");
         RETURN(0);
      }
      DSET_write(gpset);
      DSET_write(gcset);
   }
          
   RETURN(1);
}
static char shelp_GenPriors[] = {
"3dGenPriors produces classification priors based on voxel signatures.\n"
"At this stage, its main purpose is to speed up the performance of\n"
"3dSignatures when using the probablilistic method as opposed to SVM.\n"
"\n"
"Example:\n"
"3dGenPriors      -sig sigs+orig \\n"
"                 -tdist train.niml.td \\n"
"                 -pprefix anat.p \\n"
"                 -cprefix anat.c   \\n"
"                 -labeltable DSC.niml.lt \\n"
"                 -do pc   \n"
"\n"
"Options:\n"
"   -sig SIGS: Signatures dataset. A dataset with F features per voxel.\n"
"   -tdist TDIST: Training results. This file is generated by 3dSignatures.\n"
"                 ONLY training files generated by 3dSignatures' method 'prob'\n"
"                 can be used by this program. The number of features in this\n"
"                 file should match the number of features (F) in SIGS\n"
"                 This file also contains the names of the K classes that\n"
"                 will be references in the output datasets\n"
"\n"
"   -prefix PREF: Specify root prefix and let program suffix it for output \n"
"                 Volumes. This way you need not use the -*prefix options\n"
"                 below.\n"
"   -pprefix PPREF: Prefix for probability dset\n"
"   -cprefix CPREF: Prefix for class dset\n"
"   If you use -regroup_classes then you can also specify:\n"
"     -pgprefix PGPREF, and -cgprefix CGPREF\n"
"   -labeltable LTFILE: Labeltable to attach to output dset\n"
"                       This labeltable should contain all the classes\n"
"                       in TDIST\n"
"   -cmask CMASK: Provide cmask expression. Voxels where expression is 0\n"
"                 are excluded from computations\n"
"   -mask MASK: Provide mask dset\n"
"   -mrange M0 M1: Consider MASK only for values between M0 and M1, inclusive\n"
"   -do WHAT: Specify the output that this program should create.\n"
"             Each character in WHAT specifies an output. \n"
"             a 'c' produces the most likely class\n"
"             a 'p' produces probability of belonging to a class\n"
"             'pc' produces both of the above and that is the default.\n"
"             You'd be deranged to use anything else at the moment.\n"
"   -debug DBG: Set debug level\n"
"   -vox_debug 1D_DBG_INDEX: 1D index of voxel to debug.\n"
"       OR\n"
"   -vox_debug I J K: where I, J, K are the 3D voxel indices \n"
"                     (not RAI coordinates in mm)\n"
"   -vox_debug_file DBG_OUTPUT_FILE: File in which debug information is output\n"
"   -uid UID : User identifier string. It is used to generate names for\n"
"              temporary files to speed up the process. \n"
"              You must use different UID for different subjects otherwise\n"
"              you will run the risk of using bad temporary files.\n"
"              By default, uid is set to a random string.\n"
"   -use_tmp: Use temporary storage to speed up the program (see -uid )\n"
"             This is the default\n"
"   -no_tmp: Opposite of use_tmp\n"
"   -pset PSET: Reuse probability output from an earlier run.\n"
"   -cset CSET: Reuse classification output from an earlier run.\n"
"   -regroup_classes 'C1 C2 C3': Regroup classes into parent classes C1 C2 C3\n"
"                                For this to work, the original classes must \n"
"                                be named something like C1.*, C2.*, etc.\n"
"        This option can be used to replace @RegroupLabels script.\n"
"        For example:\n"
"        3dGenPriors      -sig sigs+orig \\\n"
"                         -tdist train.niml.td \\\n"
"                         -pprefix anat.p \\\n"
"                         -cprefix anat.c   \\\n"
"                         -labeltable DSC.niml.lt \\\n"
"                         -do pc   \\\n"
"                         -regroup_classes  'CSF GM WM Out'\n"
"\n"
"    or if you have the output already, you can do:\n"
"\n"
"       3dGenPriors      -sig sigs+orig \\\n"
"                         -tdist train.niml.td \\\n"
"                         -pset anat.p \\\n"
"                         -cset anat.c   \\\n"
"                         -labeltable DSC.niml.lt \\\n"
"                         -do pc   \\\n"
"                         -regroup_classes  'CSF GM WM Out'\n"           
"  -classes 'C1 C2 C3': Classify into these classes only. Alternative is\n"
"                       to classify from all the classes in the training data\n"
"  -features 'F1 F2 F3 ...': Use these features only. Otherwise use all \n"
"                            features in the training data.\n"
"                            Note that partial matching is used to resolve\n"
"                            which features to keep from training set.\n"
"  -ShowThisDist DIST: Show information obtained from the training data about\n"
"                      the distribution of DIST. For example: -\n"
"                       -ShowThisDist 'd(mean.20_mm|PER02)'\n"
"                      Set DIST to ALL to see them all.\n"
"\n"
};


void GenPriors_usage(int detail) 
{
   int i = 0;
   
   ENTRY("GenPriors_usage");
   
   
   printf( "%s", shelp_GenPriors );
   EXRETURN;
}

SEG_OPTS *GenPriors_Default(char *argv[], int argc) 
{
   SEG_OPTS *Opt=NULL;
   
   ENTRY("GenPriors_Default");
   
   Opt = SegOpt_Struct();
   
   Opt->helpfunc = &GenPriors_usage;
   Opt->ps = SUMA_Parse_IO_Args(argc, argv, "-talk;");
   Opt->aset_name = NULL;
   Opt->mset_name = NULL;
   Opt->sig_name = NULL;
   Opt->this_pset_name = NULL;
   Opt->this_cset_name = NULL;
   Opt->ndist_name = NULL;
   Opt->uid[0] = '\0';
   Opt->prefix = NULL;
   Opt->aset = NULL;
   Opt->mset = NULL;
   Opt->gset = NULL;
   Opt->sig = NULL;
   Opt->FDV = NULL;
   Opt->pset = NULL;
   Opt->cset = NULL;
   Opt->debug = 0;
   Opt->idbg = Opt->kdbg = Opt->jdbg = -1;
   Opt->binwidth = 0.01; /* the R function area.gam was used to pick a decent 
                            binwidth. I picked a large one where discrepancy
                            between Reference and Approximation was good. 
                            0.1 is too coarse, 0.001 is overkill*/ 
   Opt->feats=Opt->clss=NULL;
   Opt->keys = NULL;
   Opt->mixfrac=NULL;
   Opt->UseTmp = 1; 
   Opt->logp = 1;
   Opt->VoxDbg = -1;
   Opt->VoxDbgOut = stdout;
   Opt->rescale_p = 1;
   Opt->openmp = 0;
   Opt->labeltable_name = NULL;
   Opt->smode = STORAGE_BY_BRICK;
   Opt->pweight = 1;
   Opt->cmask = NULL;
   Opt->dimcmask = 0;
   Opt->cmask_count=0;
   Opt->mask_bot = 1.0;
   Opt->mask_top = -1.0;
   Opt->DO_p = TRUE;
   Opt->DO_c = TRUE;
   Opt->Writepcg_G_au = FALSE;
   Opt->DO_r = FALSE;
   Opt->group_classes = NULL;
   Opt->group_keys = NULL;
   Opt->fitmeth = SEG_LSQFIT;
   Opt->proot = "GenPriors";
   Opt->cs = NULL;
   Opt->Gcs = NULL;
   
   Opt->ShowThisDist = NULL;
   
   RETURN(Opt);
}

int GenPriors_CheckOpts(SEG_OPTS *Opt) 
{
   ENTRY("GenPriors_CheckOpts");
   if (  !Opt->sig_name || 
          !Opt->ndist_name  ) {
      ERROR_message("Missing input");
      RETURN(0);
   }
   
   if (Opt->group_keys && !Opt->group_classes) {
      ERROR_message("Keys but no classes");
      RETURN(0);
   }
   RETURN(1);
}

SEG_OPTS *GenPriors_ParseInput (SEG_OPTS *Opt, char *argv[], int argc)
{
   static char FuncName[]={"GenPriors_ParseInput"}; 
   int kar, i, ind, exists;
   char *outname, cview[10];
   int brk = 0;
   SUMA_GENERIC_ARGV_PARSE *ps=NULL;

   ENTRY("GenPriors_ParseInput");
   
   brk = 0;
   kar = 1;
	while (kar < argc) { /* loop accross command ine options */
		/*fprintf(stdout, "%s verbose: Parsing command line...\n", FuncName);*/
		if (strcmp(argv[kar], "-h") == 0 || strcmp(argv[kar], "-help") == 0) {
			 Opt->helpfunc(0);
          exit (0);
		}
      
 		SUMA_SKIP_COMMON_OPTIONS(brk, kar);
     
      #ifdef USE_TRACING
            if( strncmp(argv[kar],"-trace",5) == 0 ){
               DBG_trace = 1 ;
               brk = 1 ;
            }
            if( strncmp(argv[kar],"-TRACE",5) == 0 ){  
               DBG_trace = 2 ;
               brk = 1 ;
            }
      #endif
      
      if (!brk && (strcmp(argv[kar], "-debug") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -debug \n");
				exit (1);
			}
			Opt->debug = atoi(argv[kar]);
         brk = 1;
		}      
      
      if (!brk && (strcmp(argv[kar], "-save_extra") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need an argument after -save_extra \n");
				exit (1);
			}
         SUMA_S_Warn("Fix that ugly option");
			Opt->Writepcg_G_au = TRUE;
         brk = 1;
		}      
      
      if (!brk && (strcmp(argv[kar], "-talk_afni") == 0)) {
         Opt->ps->cs->talk_suma = 1;
         brk = 1;
		}      
      
      if (!brk && (strcmp(argv[kar], "-do") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -do \n");
				exit (1);
			}
			if (strchr(argv[kar], 'c')) Opt->DO_c = 1;
			if (strchr(argv[kar], 'f')) Opt->DO_f = 1;
			if (strchr(argv[kar], 'x')) Opt->DO_x = 1;
			if (strchr(argv[kar], 'p')) Opt->DO_p = 1;
         
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-L2") == 0)) {
			Opt->fitmeth = SEG_LSQFIT;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-L1") == 0)) {
			Opt->fitmeth = SEG_L1FIT;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-openmp") == 0)) {
			Opt->openmp = 1;
         brk = 1;
		}   
         
      if (!brk && (strcmp(argv[kar], "-no_openmp") == 0)) {
			Opt->openmp = 0;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-pweight") == 0)) {
			Opt->pweight = 1;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-no_pweight") == 0)) {
			Opt->pweight = 0;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-no_edge") == 0)) {
			Opt->edge = 0;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-edge") == 0)) {
			Opt->edge = 1;
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-edge1") == 0)) {
			Opt->edge = 1;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-edge2") == 0)) {
			Opt->edge = 2;
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-vox_debug") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need 1D vox index after -vox_debug \n");
				exit (1);
			}
         if (kar+2<argc) { /* see if we have ijk */
            int iii, jjj, kkk;
            if (  argv[kar  ][0]!='-' && 
                  argv[kar+1][0]!='-' && 
                  argv[kar+2][0]!='-' &&
                (iii = atoi(argv[kar  ])) >= 0 &&
                (jjj = atoi(argv[kar+1])) >= 0 && 
                (kkk = atoi(argv[kar+2])) >= 0 ) {
               Opt->VoxDbg3[0]=iii;
               Opt->VoxDbg3[1]=jjj;
               Opt->VoxDbg3[2]=kkk;    
               ++kar; ++kar;
            } 
         }
			if (Opt->VoxDbg3[0] < 0) {
            Opt->VoxDbg = atoi(argv[kar]);
         }
         brk = 1;
		}      

      if (!brk && (strcmp(argv[kar], "-vox_debug_file") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need filename after -vox_debug_file \n");
				exit (1);
			}
			Opt->VoxDbgOut = fopen(argv[kar],"w");
         brk = 1;
		}      
      
      if (strcmp(argv[kar],"-logp") == 0 ) {
         Opt->logp = 1;
         brk = 1;
      }
      
      if (strcmp(argv[kar],"-p") == 0 ) {
         Opt->logp = 0;
         brk = 1;
      }
      
      if( strcmp(argv[kar],"-use_tmp") == 0 ){
         Opt->UseTmp = 1 ;
         brk = 1;
      }

      if( strcmp(argv[kar],"-no_tmp") == 0 ){
         Opt->UseTmp = 0 ;
         brk = 1;
      }
      
      if (!brk && (strcmp(argv[kar], "-vox_debug") == 0)) {
         kar ++;
			if (kar+2 >= argc)  {
		  		fprintf (stderr, "need 3 arguments after -vox_debug \n");
				exit (1);
			}
			Opt->idbg = atoi(argv[kar]); ++kar;
         Opt->jdbg = atoi(argv[kar]); ++kar;
         Opt->kdbg = atoi(argv[kar]);
         brk = 1;
		} 
     
      if (!brk && (strcmp(argv[kar], "-cmask") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		ERROR_exit("-cmask option requires a following argument!\n");
			}
			Opt->cmask = EDT_calcmask( argv[kar] , &(Opt->dimcmask), 0 ) ;
         if( Opt->cmask == NULL ) ERROR_exit("Can't compute -cmask!\n");
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-mask") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -mask \n");
				exit (1);
			}
			Opt->mset_name = argv[kar];
         brk = 1;
      }      
      
      if( !brk && (strncmp(argv[kar],"-mrange",5) == 0) ){
         if( kar+2 >= argc )
           ERROR_exit("-mrange option requires 2 following arguments!\n");
         Opt->mask_bot = strtod( argv[++kar] , NULL ) ;
         Opt->mask_top = strtod( argv[++kar] , NULL ) ;
         if( Opt->mask_top < Opt->mask_bot )
           ERROR_exit("-mrange inputs are illegal!\n") ;
         brk = 1;
      }
      
      if (!brk && (strcmp(argv[kar], "-anat") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -anat \n");
				exit (1);
			}
			Opt->aset_name = argv[kar];
         brk = 1;
		}
            
      if (!brk && (strcmp(argv[kar], "-sig") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -sig \n");
				exit (1);
			}
			while (kar < argc && argv[kar][0] != '-') { 
            Opt->sig_name = 
               SUMA_append_replace_string(Opt->sig_name, argv[kar], " ", 1);
            ++kar;
         }
         if (kar < argc && argv[kar][0] == '-') --kar; /* unwind */
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-pset") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -pset \n");
				exit (1);
			}
			Opt->this_pset_name = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-gold") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -gold \n");
				exit (1);
			}
			Opt->gold_name = argv[kar];
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-gold_bias") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -gold_bias \n");
				exit (1);
			}
			Opt->gold_bias_name = argv[kar];
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-pstCgALL") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -pstCgALL \n");
				exit (1);
			}
			Opt->pstCgALLname = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-priCgL") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -priCgL \n");
				exit (1);
			}
			Opt->priCgLname = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-priCgALL") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -priCgALL \n");
				exit (1);
			}
			Opt->priCgALLname = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-wL") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -wL \n");
				exit (1);
			}
			Opt->wL = atof(argv[kar]);
         if (Opt->wL < 0.0 || Opt->wL > 1.0) {
            SUMA_S_Errv("-wL must be between 0 and 1.0, have %s", argv[kar]);
            exit(1);
         }
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-priCgA") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -priCgA \n");
				exit (1);
			}
			Opt->priCgAname = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-wA") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -wA \n");
				exit (1);
			}
			Opt->wA = atof(argv[kar]);
         if (Opt->wA < 0.0 || Opt->wA > 1.0) {
            SUMA_S_Errv("-wA must be between 0 and 1.0, have %s", argv[kar]);
            exit(1);
         }
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-cset") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -cset \n");
				exit (1);
			}
			Opt->this_cset_name = argv[kar];
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-fset") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -fset \n");
				exit (1);
			}
			Opt->this_fset_name = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-xset") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -xset \n");
				exit (1);
			}
			Opt->this_xset_name = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-tdist") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -tdist \n");
				exit (1);
			}
			Opt->ndist_name = argv[kar];
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-labeltable") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -labeltable \n");
				exit (1);
			}
			Opt->labeltable_name = argv[kar];
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-sphere_hood") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -sphere_hood \n");
				exit (1);
			}
			Opt->na = atof(argv[kar]);
         brk = 1;
		} 
      
      if (!brk && (strcmp(argv[kar], "-blur_meth") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -blur_meth \n");
				exit (1);
			}
			if (!strcmp(argv[kar],"BIM")) Opt->blur_meth = SEG_BIM;
         else if (!strncmp(argv[kar],"LS",2)) Opt->blur_meth = SEG_LSB;
         else if (!strcmp(argv[kar],"BNN")) Opt->blur_meth = SEG_BNN;
         else if (!strcmp(argv[kar],"BFT")) Opt->blur_meth = SEG_BFT;
         else {
            SUMA_S_Errv("-blur_meth %s not valid\n", argv[kar]);
            exit(1);
         }
         brk = 1;
		}
            
      if (!brk && (strcmp(argv[kar], "-prefix") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -prefix \n");
				exit (1);
			}
			Opt->smode = storage_mode_from_filename(argv[kar]);
         Opt->proot = argv[kar];
         Opt->prefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         Opt->crefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         Opt->pgrefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         Opt->cgrefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         Opt->frefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         Opt->xrefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         sprintf(Opt->prefix,"%s.p", argv[kar]);
         sprintf(Opt->pgrefix,"%s.pg", argv[kar]);
         sprintf(Opt->crefix,"%s.c", argv[kar]);
         sprintf(Opt->cgrefix,"%s.cg", argv[kar]);
         sprintf(Opt->frefix,"%s.f", argv[kar]);
         sprintf(Opt->xrefix,"%s.x", argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-pprefix") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -pprefix \n");
				exit (1);
			}
			Opt->smode = storage_mode_from_filename(argv[kar]);
         Opt->prefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         sprintf(Opt->prefix,"%s", argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-fprefix") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -fprefix \n");
				exit (1);
			}
			Opt->smode = storage_mode_from_filename(argv[kar]);
         Opt->frefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         sprintf(Opt->frefix,"%s", argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-cprefix") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -cprefix \n");
				exit (1);
			}
			Opt->smode = storage_mode_from_filename(argv[kar]);
         Opt->crefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         sprintf(Opt->crefix,"%s", argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-cgprefix") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -cgprefix \n");
				exit (1);
			}
			Opt->smode = storage_mode_from_filename(argv[kar]);
         Opt->cgrefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         sprintf(Opt->cgrefix,"%s", argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-pgprefix") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -pgprefix \n");
				exit (1);
			}
			Opt->smode = storage_mode_from_filename(argv[kar]);
         Opt->pgrefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         sprintf(Opt->pgrefix,"%s", argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-xprefix") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -xprefix \n");
				exit (1);
			}
			Opt->smode = storage_mode_from_filename(argv[kar]);
         Opt->xrefix = (char*)calloc(strlen(argv[kar])+20, sizeof(char));
         sprintf(Opt->xrefix,"%s", argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-bias_classes") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -bias_classes \n");
				exit (1);
			}
			Opt->bias_classes = argv[kar];
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-group_classes") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -group_classes \n");
				exit (1);
			}
			Opt->group_classes = NI_strict_decode_string_list(argv[kar] ,";");
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-group_keys") == 0)) {
         NI_str_array *nstr=NULL; int ii;
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -group_keys \n");
				exit (1);
			}
			if (!(nstr = NI_strict_decode_string_list(argv[kar] ,";, "))){
            ERROR_exit("Bad option %s after -group_keys", argv[kar]);
         }
         Opt->group_keys = (int *)calloc(nstr->num, sizeof(int));
         for (ii=0;ii<nstr->num; ++ii) 
            Opt->group_keys[ii] = strtol(nstr->str[ii],NULL,10);
         NI_delete_str_array(nstr);nstr=NULL;
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-classes") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -classes \n");
				exit (1);
			}
			Opt->clss = NI_strict_decode_string_list(argv[kar] ,";, ");
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-features") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -features \n");
				exit (1);
			}
			Opt->feats = NI_strict_decode_string_list(argv[kar] ,";, ");
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-split_classes") == 0)) {
         NI_str_array *nstr=NULL; int ii;
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -split_classes \n");
				exit (1);
			}
			nstr = NI_strict_decode_string_list(argv[kar] ,";, ");
         Opt->Split = (int *)calloc(nstr->num+1, sizeof(int));
         for (ii=0;ii<nstr->num; ++ii) {
            Opt->Split[ii] = strtol(nstr->str[ii],NULL,10);
            if (Opt->Split[ii]<1 || Opt->Split[ii]>9) {
               SUMA_S_Errv("Bad split value of %d in %s\n", 
                           Opt->Split[ii], argv[kar]);
               exit(1);
            }
         }
         Opt->Split[nstr->num]=-1; /* plug */
         
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-other") == 0)) {
         Opt->Other = 1;
         brk = 1;
      }
      
      if (!brk && (strcmp(argv[kar], "-no_other") == 0)) {
         Opt->Other = 0;
         brk = 1;
      }
      
      if (!brk && (strcmp(argv[kar], "-keys") == 0)) {
         NI_str_array *nstr=NULL; int ii;
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -keys \n");
				exit (1);
			}
			if (!(nstr = NI_strict_decode_string_list(argv[kar] ,";, "))){
            ERROR_exit("Bad option %s after -keys", argv[kar]);
         }
         Opt->keys = (int *)calloc(nstr->num, sizeof(int));
         for (ii=0;ii<nstr->num; ++ii) 
            Opt->keys[ii] = strtol(nstr->str[ii],NULL,10);
         NI_delete_str_array(nstr);nstr=NULL;
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-bias_order") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need integer after -bias_order \n");
				exit (1);
			}
			Opt->bias_param = atof(argv[kar]);
         Opt->bias_meth = "Poly";
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-bias_fwhm") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need integer after -bias_fwhm \n");
				exit (1);
			}
			Opt->bias_param = atof(argv[kar]);
         Opt->bias_meth = "Wells";
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-enhance_cset_init") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need integer after -enhance_cset_init \n");
				exit (1);
			}
			Opt->N_enhance_cset_init = atoi(argv[kar]);
         SUMA_S_Err("Option not in use at the moment");
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-main_N") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need integer after -main_N \n");
				exit (1);
			}
			Opt->N_main = atoi(argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-clust_cset_init") == 0)) {
			Opt->clust_cset_init = 1;
         brk = 1;
      }
      
      if (!brk && (strcmp(argv[kar], "-no_clust_cset_init") == 0)) {
			Opt->clust_cset_init = 0;
         brk = 1;
      }
      
      if (!brk && (strcmp(argv[kar], "-uid") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -uid \n");
				exit (1);
			}
			snprintf(Opt->uid,128,"%s",argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-mixfrac") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need argument after -mixfrac \n");
				exit (1);
			}
			Opt->mixopt = argv[kar];
         brk = 1;
		}

      if (!brk && (strcmp(argv[kar], "-Bmrf") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need value after -Bmrf \n");
				exit (1);
			}
			Opt->B = atof(argv[kar]);
         brk = 1;
		}
      
      if (!brk && (strcmp(argv[kar], "-ShowThisDist") == 0)) {
         kar ++;
			if (kar >= argc)  {
		  		fprintf (stderr, "need distname after -ShowThisDist \n");
				exit (1);
			}
			Opt->ShowThisDist = argv[kar];
         brk = 1;
		}
      if (!brk) {
			fprintf (stderr,"Option %s not understood. \n"
                         "Try -help for usage\n", argv[kar]);
			suggest_best_prog_option(argv[0], argv[kar]);
         exit (1);
		} else {	
			brk = 0;
			kar ++;
		}

   }
   
   if (!Opt->prefix) Opt->prefix = strdup("./GenPriorsOut.p");
   if (!Opt->frefix) Opt->frefix = strdup("./GenPriorsOut.f");
   if (!Opt->xrefix) Opt->xrefix = strdup("./GenPriorsOut.x");
   if (!Opt->crefix) Opt->crefix = strdup("./GenPriorsOut.c");
   if (Opt->uid[0]=='\0') UNIQ_idcode_fill(Opt->uid);

   RETURN(Opt);
}

int main(int argc, char **argv)
{
   static char FuncName[]={"3dGenPriors"};
   SEG_OPTS *Opt=NULL;
   char *atr=NULL;
   int i=0, j=0, nfound=0;
   NI_str_array *nisa = NULL, *allclss=NULL, *allfeats=NULL;
   SUMA_Boolean LocalHead = NOPE;

   SUMA_STANDALONE_INIT;
	SUMA_mainENTRY;
   
   SUMAg_DOv = SUMA_Alloc_DisplayObject_Struct (SUMA_MAX_DISPLAYABLE_OBJECTS);
   Opt = GenPriors_Default(argv,  argc);
   Opt = GenPriors_ParseInput (Opt, argv,  argc);
   
   if (!GenPriors_CheckOpts(Opt)) {
      ERROR_exit("Failed on option check");
   }
   
   if (Opt->ndist_name) {
      Opt->FDV = SUMA_get_all_dists(Opt->ndist_name); 
      if (!(allclss = SUMA_dists_classset(Opt->FDV))) {
         SUMA_S_Err("No class set?");
         exit(1);
      }
      if (!(allfeats = SUMA_dists_featureset(Opt->FDV))) {
         SUMA_S_Err("No feature set?");
         exit(1);
      }
      SUMA_S_Notev("Have total of %d features, %d classes from training\n",
                   allfeats->num, allclss->num);
   }
   
   if (Opt->ShowThisDist) {
      if (!strcmp(Opt->ShowThisDist, "ALL")) {
         SUMA_Show_dists(Opt->FDV, NULL, 1);
      } else {
         SUMA_Show_dist(
            SUMA_find_feature_dist( Opt->FDV, 
                                 Opt->ShowThisDist, NULL, NULL, NULL),
                     NULL);
      }
      exit(0); 
   }
   /* load the input data */   
   if (Opt->sig_name && !(Opt->sig = Seg_load_dset( Opt->sig_name ))) {      
      exit(1);
   }
   
   /* Fix VoxDbg */
   if (Opt->VoxDbg >= 0) {
      Vox1D2Vox3D(Opt->VoxDbg, 
                  DSET_NX(Opt->sig), DSET_NX(Opt->sig)*DSET_NY(Opt->sig),
                  Opt->VoxDbg3);
   } else if (Opt->VoxDbg3[0]>=0) {
      Opt->VoxDbg = Opt->VoxDbg3[0] + Opt->VoxDbg3[1]*DSET_NX(Opt->sig) +
                        Opt->VoxDbg3[2]*DSET_NX(Opt->sig)*DSET_NY(Opt->sig);
   }

   if (Opt->VoxDbg < 0 || Opt->VoxDbg >= DSET_NVOX(Opt->sig)) {
      SUMA_S_Errv("Voxel debug %d (%d %d %d) outside of grid for Opt->sig\n",
               Opt->VoxDbg, Opt->VoxDbg3[0], Opt->VoxDbg3[1], Opt->VoxDbg3[2]);
      exit(1);
   }
   

   
   if (!Opt->clss) { /* get all classes from training */
      Opt->clss = allclss; allclss = NULL;
   } else {
      nisa = NULL;
      for (j=0; j<Opt->clss->num; ++j) {
         nfound=0;
         for (i=0; i<allclss->num; ++i) {
            if (strstr(allclss->str[i], Opt->clss->str[j])) {
               nisa = SUMA_NI_str_array(nisa, allclss->str[i], "A"); 
               ++nfound;
            }
         }
         if (!nfound) {
            SUMA_S_Errv("Class %s has no match in training set\n",
                        Opt->clss->str[j]);
            exit(1);
         }
      }
      if (!nisa || nisa->num < 1) {
         SUMA_S_Err("Found 0 classes in training set "
                    "from list on command line.");
         exit(1);
      }
      if (nisa->num < allclss->num) {
         SUMA_S_Notev("%d classes selected out of "
                      "%d classes in training set.\n",
                      nisa->num, allclss->num);
      }
      Opt->clss = SUMA_free_NI_str_array(Opt->clss); 
      Opt->clss = nisa; nisa = NULL;
      #if 0
      /* make sure we have all classes represented in training */
      for (i=0; i<Opt->clss->num; ++i) {
         if (NI_str_array_find(Opt->clss->str[i], allclss) < 0) {
            SUMA_S_Errv("Class %s not in training group!\n", Opt->clss->str[i]);
            exit(1);
         }
      }
      #endif
   }
   
   if (!Opt->feats) { /* get all features from input signatures */
      for (i=0; i<DSET_NVALS(Opt->sig); ++i) {
         Opt->feats = SUMA_NI_str_array(Opt->feats, 
                                        DSET_BRICK_LABEL(Opt->sig,i),"A");
      }
   } 
   if (!Opt->feats) { /* Should not happen here */
      SUMA_S_Err("No features by this point?!?");
      exit(1);
   } else {
      nisa = NULL;
      for (j=0; j<Opt->feats->num; ++j) {
         nfound = 0;
         for (i=0; i<allfeats->num; ++i) {
            if (strstr(allfeats->str[i], Opt->feats->str[j])) {
               nisa = SUMA_NI_str_array(nisa, allfeats->str[i], "A"); 
               ++nfound;
            }
         }
         if (!nfound) {
            SUMA_S_Errv("Feature %s has no match in training set\n",
                        Opt->feats->str[j]);
            exit(1);
         }
      }
      if (!nisa || nisa->num < 1) {
         SUMA_S_Err("Found 0 features in training set "
                    "from list on command line.");
         exit(1);
      }
      if (nisa->num < allfeats->num) {
         SUMA_S_Notev("%d features selected out of "
                      "%d features in training set.\n",
                      nisa->num, allfeats->num);
      }
      Opt->feats = SUMA_free_NI_str_array(Opt->feats); 
      Opt->feats = nisa; nisa = NULL;
      #if 0
      /* make sure we have all features represented in training*/
      for (i=0; i<Opt->feats->num; ++i) {
         if (SUMA_NI_str_array_find(Opt->feats->str[i], allfeats, 1, 0) < 0) {
            SUMA_S_Errv("Feature %s not in training group!\n", 
                        Opt->feats->str[i]);
            exit(1);
         }
      }
      #endif
   }
   SUMA_S_Notev("Will be using %d features and %d classes.\n",
                   Opt->feats->num, Opt->clss->num);
   
   /* make sure we have mixfrac */
   if (!Opt->mixfrac) {
      SUMA_S_Note("Using Equal Mixing Fractions");
      Opt->mixfrac = (float*)calloc(Opt->clss->num, sizeof(float));
      for (i=0; i<Opt->clss->num; ++i) {
         Opt->mixfrac[i] = 1/(float)Opt->clss->num;
      }
   } else {
      /* force one for the moment */
      ERROR_exit("Cannot handle user mixfrac yet");
   }
   
   if (Opt->mset_name) {
      if (!(Opt->mset = Seg_load_dset( Opt->mset_name ))) {      
         exit(1);
      }
   }

   if (Opt->this_pset_name) {
      SUMA_S_Notev("Reading pre-existing %s\n",Opt->this_pset_name);
      if (!(Opt->pset = Seg_load_dset( Opt->this_pset_name ))) {      
         exit(1);
      }
   }
   
   if (Opt->this_cset_name) {
      SUMA_S_Notev("Reading pre-existing %s\n",Opt->this_cset_name);
      if (!(Opt->cset = Seg_load_dset( Opt->this_cset_name ))) {      
         exit(1);
      }
      #if 0
         /* you should not attempt to get clss from the cset
         because that guy might have a labeltable that contains more
         classes than in the training file. */
      if (!Opt->clss) { /* try to get it from this dset */
         if (!Seg_ClssAndKeys_from_dset(Opt->cset, &(Opt->clss), 
                                          &(Opt->clss_keys))) {
            ERROR_message("No clss to be found in cset.\n"
                          "Must specify labeltable");
            exit(1);
         }
      }
      #endif
   }


   if (!Opt->clss) {
      ERROR_message("Must specify -tdist");
      exit(1);
   }
   
   /* labeltable? */
   if (Opt->labeltable_name) {
      Dtable *vl_dtable=NULL;
      char *labeltable_str=NULL;
      int kk=0;
      
      /* read the table */
      if (!(labeltable_str = AFNI_suck_file( Opt->labeltable_name))) {
         ERROR_exit("Failed to read %s", Opt->labeltable_name);
      }
      if (!(vl_dtable = Dtable_from_nimlstring(labeltable_str))) {
         ERROR_exit("Could not parse labeltable");
      }
      /* make sure all classes are in the labeltable */
      for (i=0; i<Opt->clss->num; ++i) {
         if ((kk = SUMA_KeyofLabel_Dtable(vl_dtable, Opt->clss->str[i]))<0){
            ERROR_exit("Key not found in %s for %s ", 
                        Opt->labeltable_name, Opt->clss->str[i]);
         }
         if (Opt->keys) {
            if (Opt->keys[i]!=kk) {
               ERROR_exit("Key mismatch %d %d", Opt->keys[i], kk);
            }
         }   
      }   
      if (!Opt->keys) { /* get them from table */
         Opt->keys = (int *)calloc(Opt->clss->num, sizeof(int));
         for (i=0; i<Opt->clss->num; ++i) {
            if ((kk = SUMA_KeyofLabel_Dtable(vl_dtable, Opt->clss->str[i]))<0){
               ERROR_exit("(should noy happen) Key not found in %s for %s ", 
                           Opt->labeltable_name, Opt->clss->str[i]);
            }
            Opt->keys[i] = kk;
         }
      }
      destroy_Dtable(vl_dtable); vl_dtable=NULL;
   } 
   
   if (!Opt->keys) {
      /* add default keys */
      SUMA_S_Note("Keys not available, assuming defaults");
      Opt->keys = (int *)calloc(Opt->clss->num, sizeof(int));
      for (i=0; i<Opt->clss->num; ++i) {
         Opt->keys[i] = i+1;
      }
   }
   
   /* Show the match between keys and classes */
   SUMA_ShowClssKeys(Opt->clss->str, Opt->clss->num, Opt->keys);
   
   
   
   if (Opt->group_classes) { /* just a check */
      if (!SUMA_Regroup_classes (Opt, 
                     Opt->clss->str, Opt->clss->num, Opt->keys,
                     Opt->group_classes->str, 
                     Opt->group_classes->num, NULL,
                     NULL, NULL, NULL, 
                     NULL, NULL)) {
         ERROR_exit("Failed to determine mapping");
      }
   }
   
   
   if (Opt->sig) {
      Opt->cmask = MaskSetup(Opt, Opt->sig, 
                &(Opt->mset), &(Opt->cmask), Opt->dimcmask, 
                Opt->mask_bot, Opt->mask_top, &(Opt->cmask_count));
   }
   
   
   if (Opt->VoxDbg >= 0) {
      fprintf(Opt->VoxDbgOut, "Command:");
      for (i=0; i<argc; ++i) {
         fprintf(Opt->VoxDbgOut, "%s ", argv[i]);
      }
      fprintf(Opt->VoxDbgOut, "\nDebug info for voxel %d (%d %d %d)\n", 
               Opt->VoxDbg, Opt->VoxDbg3[0], Opt->VoxDbg3[1], Opt->VoxDbg3[2]);
   }

   /* An inportant sanity check */
   if (Opt->pset && Opt->clss->num != DSET_NVALS(Opt->pset)) {
      ERROR_exit( "Number of classes %d does not "
                  "match number of pset subricks %d\n",
                  Opt->clss->num , DSET_NVALS(Opt->pset));
   }
   
   if (!GenPriors(Opt)) {
      ERROR_exit("Failed in GenPriors");
   }
   
   /* write output */
   if (Opt->pset && !Opt->this_pset_name) {
      DSET_write(Opt->pset);
   }
   if (Opt->cset && !Opt->this_cset_name) {
      DSET_write(Opt->cset);
   }
   
   /* all done, free */
   Opt = free_SegOpts(Opt);
   
   PRINT_COMPILE_DATE ; exit(0);
}
