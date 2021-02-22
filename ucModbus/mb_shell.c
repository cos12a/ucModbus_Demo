/*
*********************************************************************************************************
*                                        LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


static  CPU_INT16S  ShShell_help(CPU_INT16U        argc,
                                 CPU_CHAR         *argv[],
                                 SHELL_OUT_FNCT    out_fnct,
                                 SHELL_CMD_PARAM  *pcmd_param);

static  SHELL_CMD  MbShell_CmdTbl [] =
{
    {"mb_help", ShShell_help},
    {0,         0           }
};




/*
*********************************************************************************************************
*                                             ShShell_Init()
*
* Description : Initialize Shell for general shell commands.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if general shell commands were added.
*               DEF_FAIL, otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  ShShell_Init (void)
{
    SHELL_ERR    err;
    CPU_BOOLEAN  ok;


    Shell_CmdTblAdd((CPU_CHAR *)"sh", ShShell_CmdTbl, &err);

    ok = (err == SHELL_ERR_NONE) ? DEF_OK : DEF_FAIL;
    return (ok);
}




/*
*********************************************************************************************************
*                                             ShShell_help()
*
* Description : List all commands or invoke '--help' for another command.
*
* Argument(s) : argc            The number of arguments.
*
*               argv            Array of arguments.
*
*               out_fnct        The output function.
*
*               pcmd_param      Pointer to the command parameters.
*
* Return(s)   : SHELL_EXEC_ERR, if an error is encountered.
*               SHELL_ERR_NONE, otherwise.
*
* Caller(s)   : Shell, in response to command execution.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT16S  ShShell_help (CPU_INT16U        argc,
                                  CPU_CHAR         *argv[],
                                  SHELL_OUT_FNCT    out_fnct,
                                  SHELL_CMD_PARAM  *pcmd_param)
{
    CPU_CHAR           cmd_str[SHELL_CFG_MODULE_CMD_NAME_LEN_MAX + 4];
    SHELL_ERR          err;
    SHELL_CMD         *pcmd;
    SHELL_MODULE_CMD  *pmodule_cmd;


    if (argc == 2) {
        if (Str_Cmp(argv[1], SH_SHELL_STR_HELP) == 0) {
            (void)out_fnct(SH_SHELL_ARG_ERR_HELP, (CPU_INT16U)Str_Len(SH_SHELL_ARG_ERR_HELP), pcmd_param->pout_opt);
            (void)out_fnct(SH_SHELL_NEW_LINE,     2,                                          pcmd_param->pout_opt);
            (void)out_fnct(SH_SHELL_CMD_EXP_HELP, (CPU_INT16U)Str_Len(SH_SHELL_CMD_EXP_HELP), pcmd_param->pout_opt);
            (void)out_fnct(SH_SHELL_NEW_LINE,     2,                                          pcmd_param->pout_opt);
            return (SHELL_ERR_NONE);
        }
    }

    if ((argc != 1) && (argc != 2)) {
        (void)out_fnct(SH_SHELL_ARG_ERR_HELP, (CPU_INT16U)Str_Len(SH_SHELL_ARG_ERR_HELP), pcmd_param->pout_opt);
        (void)out_fnct(SH_SHELL_NEW_LINE,     2,                                          pcmd_param->pout_opt);
        return (SHELL_EXEC_ERR);
    }

    switch (argc) {
        case 1:
             pmodule_cmd = Shell_ModuleCmdUsedPoolPtr;
             while (pmodule_cmd != (SHELL_MODULE_CMD *)0) {
                 pcmd = pmodule_cmd->CmdTblPtr;
                 if (pcmd != (SHELL_CMD *)0) {
                     while (pcmd->Fnct != (SHELL_CMD_FNCT)0) {
                         (void)out_fnct((CPU_CHAR *)pcmd->Name,
                                        (CPU_INT16U)Str_Len(pcmd->Name),
                                                    pcmd_param->pout_opt);
                         (void)out_fnct(SH_SHELL_NEW_LINE, 2,                               pcmd_param->pout_opt);
                         pcmd++;
                     }
                 }
                 pmodule_cmd = pmodule_cmd->NextModuleCmdPtr;
             }
             break;

        case 2:
             Str_Copy(cmd_str, argv[1]);
             Str_Cat(cmd_str, (CPU_CHAR *)" ");
             Str_Cat(cmd_str, SH_SHELL_STR_HELP);

             Shell_Exec(cmd_str, out_fnct, pcmd_param, &err);

             switch (err) {
                case SHELL_ERR_CMD_NOT_FOUND:
                case SHELL_ERR_CMD_SEARCH:
                case SHELL_ERR_ARG_TBL_FULL:
                     (void)out_fnct((CPU_CHAR *)"Command not recognized: ", 25,                           pcmd_param->pout_opt);
                     (void)out_fnct(argv[1],                                (CPU_INT16U)Str_Len(argv[1]), pcmd_param->pout_opt);
                     (void)out_fnct(SH_SHELL_NEW_LINE,                      2,                            pcmd_param->pout_opt);
                     break;

                case SHELL_ERR_NONE:
                case SHELL_ERR_NULL_PTR:
                case SHELL_ERR_CMD_EXEC:
                default:
                     break;
             }
             break;

        default:
             break;
    }

    return (SHELL_ERR_NONE);
}





/****************************************************************************************************//**
 *                                           ClkCmd_OutputCmdTbl()
 *
 * @brief    Format and output the clock test command table
 *
 * @param    p_cmd_tbl       is the pointer on the pointer table
 *
 * @param    out_fnct        is a callback to a respond to the requester.
 *
 * @param    p_cmd_param     is a pointer to additional information to pass to the command.
 *
 * @return   The number of positive data octets transmitted, if NO error(s).
 *               - SHELL_OUT_RTN_CODE_CONN_CLOSED, if implemented connection closed.
 *               - SHELL_OUT_ERR, otherwise.
 *******************************************************************************************************/
static CPU_INT16S ClkCmd_OutputCmdTbl(SHELL_CMD       *p_cmd_tbl,
                                      SHELL_OUT_FNCT  out_fnct,
                                      SHELL_CMD_PARAM *p_cmd_param)
{
  SHELL_CMD  *p_shell_cmd;
  CPU_INT16S ret_val;
  CPU_INT16S acc_ret_val;

  ret_val = ClkCmd_OutputMsg(CLK_CMD_OUTPUT_CMD_LIST,
                             DEF_YES,
                             DEF_YES,
                             DEF_NO,
                             out_fnct,
                             p_cmd_param);
  switch (ret_val) {
    case SHELL_OUT_RTN_CODE_CONN_CLOSED:
    case SHELL_OUT_ERR:
      return (SHELL_EXEC_ERR);

    default:
      break;
  }

  acc_ret_val = ret_val;
  p_shell_cmd = p_cmd_tbl;

  while (p_shell_cmd->Fnct != 0) {
    ret_val = ClkCmd_OutputMsg(p_shell_cmd->Name,
                               DEF_NO,
                               DEF_YES,
                               DEF_YES,
                               out_fnct,
                               p_cmd_param);
    switch (ret_val) {
      case SHELL_OUT_RTN_CODE_CONN_CLOSED:
      case SHELL_OUT_ERR:
        return (SHELL_EXEC_ERR);

      default:
        break;
    }
    p_shell_cmd++;
    acc_ret_val += ret_val;
  }

  return (acc_ret_val);
}





/********************************************************************************************************
 ********************************************************************************************************
 *                                           PRIVATE FUNCTIONS
 ********************************************************************************************************
 *******************************************************************************************************/

/****************************************************************************************************//**
 *                                           Shell_ListCmdOutput()
 *
 * @brief    Outputs a list of all commands.
 *
 * @param    out_fnct        The output function.
 *
 * @param    p_cmd_param     Pointer to the command parameters.
 *
 * @note     (1) This function must only be called with the Shell lock acquired.
 *******************************************************************************************************/
void Shell_ListCmdOutput(SHELL_OUT_FNCT  out_fnct,
                         SHELL_CMD_PARAM *p_cmd_param)
{
  SHELL_MODULE_CMD *p_module_cmd;
  SHELL_CMD        *p_cmd;

  p_module_cmd = Shell_DataPtr->ModuleCmdListHeadPtr;

  while (p_module_cmd != DEF_NULL) {
    p_cmd = p_module_cmd->CmdTblPtr;
    while (p_cmd->Fnct != DEF_NULL) {
      (void)out_fnct((CPU_CHAR *)p_cmd->Name,
                     (CPU_INT16U)Str_Len(p_cmd->Name),
                     p_cmd_param->pout_opt);
      (void)out_fnct(STR_NEW_LINE, (CPU_INT16U)Str_Len(STR_NEW_LINE), p_cmd_param->OutputOptPtr);
      p_cmd++;
    }
    p_module_cmd = p_module_cmd->NextModuleCmdPtr;
  }
}

