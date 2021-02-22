/***************************************************************************//**
 * @file
 * @brief Common - Clock - Calendar Commands
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/********************************************************************************************************
 ********************************************************************************************************
 *                                           INCLUDE FILES
 ********************************************************************************************************
 *******************************************************************************************************/

#include  <shell.h>
#include  <MB_cmd_priv.h>


/********************************************************************************************************
 ********************************************************************************************************
 *                                               LOCAL DEFINES
 ********************************************************************************************************
 *******************************************************************************************************/

#define  MB_CMD_ARG_BEGIN                  ASCII_CHAR_HYPHEN_MINUS
#define  MB_CMD_ARG_TIME_TYPE              ASCII_CHAR_LATIN_LOWER_T

#define  MB_CMD_NBR_MIN_PER_HR             60u
#define  MB_CMD_NBR_SEC_PER_MIN            60u

#define  RTOS_MODULE_CUR                    RTOS_CFG_MODULE_COMMON

/********************************************************************************************************
 ********************************************************************************************************
 *                                           LOCAL CONSTANTS
 ********************************************************************************************************
 *******************************************************************************************************/

#define  MB_CMD_HELP                                        ("help")
#define  MB_CMD_HELP_SHORT                                  ("h")

#define  MB_CMD_FORMAT_DATETIME                             ("datetime")
#define  MB_CMD_FORMAT_NTP                                  ("ntp")
#define  MB_CMD_FORMAT_UNIX                                 ("unix")

#define  MB_CMD_FORMAT_DATETIME_SHORT                       ("d")
#define  MB_CMD_FORMAT_NTP_SHORT                            ("n")
#define  MB_CMD_FORMAT_UNIX_SHORT                           ("u")

#define  MB_CMD_OUTPUT_CMD_LIST                             ("Command List: ")
#define  MB_CMD_OUTPUT_ERR                                  ("Error: ")
#define  MB_CMD_OUTPUT_SUCCESS                              ("Completed successfully")
#define  MB_CMD_OUTPUT_TABULATION                           ("\t")

#define  MB_CMD_OUTPUT_ERR_CMD_ARG_INVALID                  ("Invalid Arguments")
#define  MB_CMD_OUTPUT_ERR_CMD_NOT_IMPLEMENTED              ("This command is not yet implemented")
#define  MB_CMD_OUTPUT_ERR_CONV_DISABLED                    ("MB_CFG_STR_CONV_EN is not enabled. Formatting not available.")
#define  MB_CMD_OUTPUT_ERR_CMD_INTERNAL_ERR                 ("Mb Internal Error. Date & Time cannot be converted.")

#define  MB_CMD_HELP_SET                                    ("usage: mdbus_set [VALUE] {FORMAT}\r\n")

#define  MB_CMD_HELP_GET                                    ("usage: mdbus_get {FORMAT}\r\n")

#define  MB_CMD_HELP_VALUE                                  ("where VALUE is:\r\n"                                                 \
                                                              "  YYYY-MM-DD {HH:MM:SS {UTC+/-HH:MM}}      for DATETIME format.\r\n" \
                                                              "  a 32-bit integer greater than 946684799  for UNIX     format.\r\n" \
                                                              "  a 32-bit integer greater than 3155673599 for NTP      format.\r\n")

#define  MB_CMD_HELP_FORMAT                                 ("where FORMAT is:\r\n"                        \
                                                              "\r\n"                                        \
                                                              "  -d, --datetime   DATETIME format.\r\n"     \
                                                              "  -u, --unix       UNIX     format.\r\n"     \
                                                              "  -n, --ntp        NTP      format.\r\n\r\n" \
                                                              "  if FORMAT is not provided, VALUE is assumed to be in DATETIME format.\r\n")

/********************************************************************************************************
 ********************************************************************************************************
 *                                               DATA TYPES
 ********************************************************************************************************
 *******************************************************************************************************/

/*
 ********************************************************************************************************
 *                                       CLOCK COMMAND TIME DATA TYPE
 *******************************************************************************************************/

typedef enum mdbus_cmd_time_type {
  MB_CMD_TIME_TYPE_NONE = 0x00,
  MB_CMD_TIME_TYPE_DATETIME = 0x30,
  MB_CMD_TIME_TYPE_NTP = 0x31,
  MB_CMD_TIME_TYPE_UNIX = 0x32
} MB_CMD_TIME_TYPE;

/*
 ********************************************************************************************************
 *                                   CLOCK COMMAND ARGUMENT DATA TYPE
 *******************************************************************************************************/

typedef struct mdbus_cmd_arg {
  MB_CMD_TIME_TYPE TimeType;
  CPU_CHAR          *DatePtr;
  CPU_CHAR          *TimePtr;
  CPU_CHAR          *OffsetPtr;
} MB_CMD_ARG;

/*
 ********************************************************************************************************
 *                                   CLOCK COMMAND PARSE STATUS DATA TYPE
 *******************************************************************************************************/

typedef enum mdbus_cmd_parse_status {
  MB_CMD_PARSE_STATUS_SUCCESS,
  MB_CMD_PARSE_STATUS_EMPTY,
  MB_CMD_PARSE_STATUS_INVALID_ARG,
  MB_CMD_PARSE_STATUS_HELP
} MB_CMD_PARSE_STATUS;

/********************************************************************************************************
 ********************************************************************************************************
 *                                       LOCAL FUNCTION PROTOTYPES
 ********************************************************************************************************
 *******************************************************************************************************/

static CPU_INT16S MbCmd_Help(CPU_INT16U      argc,
                              CPU_CHAR        *p_argv[],
                              SHELL_OUT_FNCT  out_fnct,
                              SHELL_CMD_PARAM *p_cmd_param);

static CPU_INT16S MbCmd_Set(CPU_INT16U      argc,
                             CPU_CHAR        *p_argv[],
                             SHELL_OUT_FNCT  out_fnct,
                             SHELL_CMD_PARAM *p_cmd_param);

static CPU_INT16S MbCmd_Open(CPU_INT16U      argc,
                             CPU_CHAR        *p_argv[],
                             SHELL_OUT_FNCT  out_fnct,
                             SHELL_CMD_PARAM *p_cmd_param);

static CPU_INT16S MbCmd_Close(CPU_INT16U      argc,
                                    CPU_CHAR        *p_argv[],
                                    SHELL_OUT_FNCT  out_fnct,
                                    SHELL_CMD_PARAM *p_cmd_param);

static MB_CMD_PARSE_STATUS MbCmd_CmdArgParse(CPU_INT16U  argc,
                                               CPU_CHAR    *p_argv[],
                                               MB_CMD_ARG *p_cmd_args);

static CPU_INT16S MbCmd_OutputCmdTbl(SHELL_CMD       *p_cmd_tbl,
                                      SHELL_OUT_FNCT  out_fnct,
                                      SHELL_CMD_PARAM *p_cmd_param);

static CPU_INT16S MbCmd_OutputError(CPU_CHAR        *p_error,
                                     SHELL_OUT_FNCT  out_fnct,
                                     SHELL_CMD_PARAM *p_cmd_param);

static CPU_INT16S MbCmd_OutputMsg(const CPU_CHAR  *p_msg,
                                   CPU_BOOLEAN     new_line_start,
                                   CPU_BOOLEAN     new_line_end,
                                   CPU_BOOLEAN     tab_start,
                                   SHELL_OUT_FNCT  out_fnct,
                                   SHELL_CMD_PARAM *p_cmd_param);

/********************************************************************************************************
 ********************************************************************************************************
 *                                               LOCAL TABLES
 ********************************************************************************************************
 *******************************************************************************************************/

static SHELL_CMD MbCmdTbl[] =
{
  { "mb_help", MbCmd_Help },
  { "mb_set", MbCmd_Set },
  { "mb_open", MbCmd_Open },
  { "mb_close", MbCmd_Close },
  { 0, 0 }
};

/********************************************************************************************************
 ********************************************************************************************************
 *                                                   MACRO
 ********************************************************************************************************
 *******************************************************************************************************/

#define  MB_CMD_OUT_MSG_CHK(out_val, cur_out_cnt, exit_fail_label)     do { \
    switch (out_val) {                                                       \
      case SHELL_OUT_RTN_CODE_CONN_CLOSED:                                   \
      case SHELL_OUT_ERR:                                                    \
        out_val = SHELL_EXEC_ERR;                                            \
        goto exit_fail_label;                                                \
                                                                             \
                                                                             \
      default:                                                               \
        cur_out_cnt += out_val;                                              \
        break;                                                               \
    }                                                                        \
} while (0)

/********************************************************************************************************
 ********************************************************************************************************
 *                                           GLOBAL FUNCTIONS
 ********************************************************************************************************
 *******************************************************************************************************/

/****************************************************************************************************//**
 *                                               MbCmd_Init()
 *
 * @brief    Adds the Mb commands to Shell.
 *
 * @param    p_err   Pointer to the variable that will receive one of the following error code(s) from
 *                   this function:
 *                       - RTOS_ERR_NONE
 *                       - RTOS_ERR_OWNERSHIP
 *                       - RTOS_ERR_ALREADY_EXISTS
 *                       - RTOS_ERR_BLK_ALLOC_CALLBACK
 *                       - RTOS_ERR_SEG_OVF
 *                       - RTOS_ERR_OS_SCHED_LOCKED
 *                       - RTOS_ERR_NOT_AVAIL
 *                       - RTOS_ERR_POOL_EMPTY
 *                       - RTOS_ERR_WOULD_OVF
 *                       - RTOS_ERR_OS_OBJ_DEL
 *                       - RTOS_ERR_WOULD_BLOCK
 *                       - RTOS_ERR_INVALID_ARG
 *                       - RTOS_ERR_NO_MORE_RSRC
 *                       - RTOS_ERR_IS_OWNER
 *                       - RTOS_ERR_ABORT
 *                       - RTOS_ERR_TIMEOUT
 *******************************************************************************************************/
CPU_BOOLEAN  MbCmd_Init (void)
{
    SHELL_ERR    err;
    CPU_BOOLEAN  ok;


    Shell_CmdTblAdd((CPU_CHAR *)"mb", MbCmdTbl, &err);

    ok = (err == SHELL_ERR_NONE) ? DEF_OK : DEF_FAIL;
    return (ok);
}

/********************************************************************************************************
 ********************************************************************************************************
 *                                           LOCAL FUNCTIONS
 ********************************************************************************************************
 *******************************************************************************************************/

/****************************************************************************************************//**
 *                                               MbCmd_Help()
 *
 * @brief    Output the available commands.
 *
 * @param    argc            Count of the arguments supplied.
 *
 * @param    p_argv          Array of pointers to the strings which are those arguments.
 *
 * @param    out_fnct        Callback to a respond to the requester.
 *
 * @param    p_cmd_param     Pointer to additional information to pass to the command.
 *
 * @return   The number of positive data octets transmitted, if NO error(s).
 *               - SHELL_OUT_RTN_CODE_CONN_CLOSED, if implemented connection closed.
 *               - SHELL_OUT_ERR, otherwise.
 *******************************************************************************************************/
static CPU_INT16S MbCmd_Help(CPU_INT16U      argc,
                              CPU_CHAR        *p_argv[],
                              SHELL_OUT_FNCT  out_fnct,
                              SHELL_CMD_PARAM *p_cmd_param)
{
  CPU_INT16S ret_val;

//  void (argc);
//  void (p_argv);

  ret_val = MbCmd_OutputCmdTbl(MbCmdTbl,
                                out_fnct,
                                p_cmd_param);

  return (ret_val);
}

/****************************************************************************************************//**
 *                                               MbCmd_Set()
 *
 * @brief    Set the current date and time.
 *
 * @param    argc            Count of the arguments supplied.
 *
 * @param    p_argv          Array of pointers to the strings which are those arguments.
 *
 * @param    out_fnct        Callback to a respond to the requester.
 *
 * @param    p_cmd_param     Pointer to additional information to pass to the command.
 *
 * @return   The number of positive data octets transmitted, if NO error(s).
 *               - SHELL_OUT_RTN_CODE_CONN_CLOSED, if implemented connection closed.
 *               - SHELL_OUT_ERR, otherwise.
 *******************************************************************************************************/
static CPU_INT16S MbCmd_Set(CPU_INT16U      argc,
                             CPU_CHAR        *p_argv[],
                             SHELL_OUT_FNCT  out_fnct,
                             SHELL_CMD_PARAM *p_cmd_param)
{
    CPU_INT16S           ret_val = 0u;
    CPU_INT16S           byte_out_cnt = 0u;


arg_invalid:
  ret_val = MbCmd_OutputError((CPU_CHAR *)MB_CMD_OUTPUT_ERR_CMD_ARG_INVALID,
                               out_fnct,
                               p_cmd_param);

  MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);
  goto exit_ok;

exit_ok:
  ret_val = byte_out_cnt;

exit_fail:
  return (ret_val);
}

/****************************************************************************************************//**
 *                                               MbCmd_Open()
 *
 * @brief    Manage a clock get command according to the specified format.
 *
 * @param    argc            Count of the arguments supplied.
 *
 * @param    p_argv          Array of pointers to the strings which are those arguments.
 *
 * @param    out_fnct        Callback to a respond to the requester.
 *
 * @param    p_cmd_param     Pointer to additional information to pass to the command.
 *
 * @return   The number of positive data octets transmitted, if NO error(s).
 *               - SHELL_OUT_RTN_CODE_CONN_CLOSED, if implemented connection closed.
 *               - SHELL_OUT_ERR, otherwise.
 *******************************************************************************************************/
static CPU_INT16S MbCmd_Open(CPU_INT16U      argc,
                             CPU_CHAR        *p_argv[],
                             SHELL_OUT_FNCT  out_fnct,
                             SHELL_CMD_PARAM *p_cmd_param)
{

  CPU_INT16S           ret_val = 0u;
  CPU_INT16S           byte_out_cnt = 0u;
  MB_CMD_ARG          cmd_arg;
  MB_CMD_PARSE_STATUS parse_status;

  parse_status = MbCmd_CmdArgParse(argc,
                                    p_argv,
                                    &cmd_arg);

  switch (parse_status) {
    case MB_CMD_PARSE_STATUS_EMPTY:
    case MB_CMD_PARSE_STATUS_SUCCESS:
      break;

    case MB_CMD_PARSE_STATUS_INVALID_ARG:
      ret_val = MbCmd_OutputError((CPU_CHAR *)MB_CMD_OUTPUT_ERR_CMD_ARG_INVALID,
                                   out_fnct,
                                   p_cmd_param);

      MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);
      goto exit_ok;

    case MB_CMD_PARSE_STATUS_HELP:
    default:
      ret_val = MbCmd_OutputMsg(MB_CMD_HELP_GET,
                                 DEF_YES,
                                 DEF_YES,
                                 DEF_NO,
                                 out_fnct,
                                 p_cmd_param);

      MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);

      ret_val = MbCmd_OutputMsg(MB_CMD_HELP_FORMAT,
                                 DEF_YES,
                                 DEF_YES,
                                 DEF_NO,
                                 out_fnct,
                                 p_cmd_param);

      MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);
      goto exit_ok;
  }

 

date_time_invalid:
  ret_val = MbCmd_OutputError((CPU_CHAR *)MB_CMD_OUTPUT_ERR_CMD_INTERNAL_ERR,
                               out_fnct,
                               p_cmd_param);

  MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);
  goto exit_ok;

exit_ok:
  ret_val = byte_out_cnt;

exit_fail:
  return (ret_val);
}

/****************************************************************************************************//**
 *                                           MbCmd_Close()
 *
 * @brief    Manage a clock get command according to the specified format.
 *
 * @param    argc            Count of the arguments supplied.
 *
 * @param    p_argv          Array of pointers to the strings which are those arguments.
 *
 * @param    out_fnct        Callback to a respond to the requester.
 *
 * @param    p_cmd_param     Pointer to additional information to pass to the command.
 *
 * @return   The number of positive data octets transmitted, if NO error(s).
 *               - SHELL_OUT_RTN_CODE_CONN_CLOSED, if implemented connection closed.
 *               - SHELL_OUT_ERR, otherwise.
 *******************************************************************************************************/
static CPU_INT16S MbCmd_Close(CPU_INT16U      argc,
                                    CPU_CHAR        *p_argv[],
                                    SHELL_OUT_FNCT  out_fnct,
                                    SHELL_CMD_PARAM *p_cmd_param)
{
  CPU_INT16S           ret_val = 0u;
  CPU_INT16S           byte_out_cnt = 0u;

  MB_CMD_ARG          cmd_arg;
  MB_CMD_PARSE_STATUS status;


  status = MbCmd_CmdArgParse(argc,
                              p_argv,
                              &cmd_arg);
  switch (status) {
    case MB_CMD_PARSE_STATUS_SUCCESS:
      break;

    case MB_CMD_PARSE_STATUS_INVALID_ARG:
      ret_val = MbCmd_OutputError((CPU_CHAR *)MB_CMD_OUTPUT_ERR_CMD_ARG_INVALID,
                                   out_fnct,
                                   p_cmd_param);

      MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);
      goto exit_ok;

    case MB_CMD_PARSE_STATUS_EMPTY:
    case MB_CMD_PARSE_STATUS_HELP:
    default:
      ret_val = MbCmd_OutputMsg(MB_CMD_HELP_SET,
                                 DEF_YES,
                                 DEF_YES,
                                 DEF_NO,
                                 out_fnct,
                                 p_cmd_param);

      MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);

      ret_val = MbCmd_OutputMsg(MB_CMD_HELP_VALUE,
                                 DEF_YES,
                                 DEF_YES,
                                 DEF_NO,
                                 out_fnct,
                                 p_cmd_param);

      MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);

      ret_val = MbCmd_OutputMsg(MB_CMD_HELP_FORMAT,
                                 DEF_YES,
                                 DEF_YES,
                                 DEF_NO,
                                 out_fnct,
                                 p_cmd_param);

      MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);
      goto exit_ok;
  }


arg_invalid:
  ret_val = MbCmd_OutputError((CPU_CHAR *)MB_CMD_OUTPUT_ERR_CMD_ARG_INVALID,
                               out_fnct,
                               p_cmd_param);

  MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);
  goto exit_ok;

exit_ok:
  ret_val = byte_out_cnt;

exit_fail:
  return (ret_val);
}

/****************************************************************************************************//**
 *                                           MbCmd_CmdArgParse()
 *
 * @brief    Parse and validate the argument for a clock test command.
 *
 * @param    argc        Count of the arguments supplied.
 *
 * @param    p_argv      Array of pointers to the strings which are those arguments.
 *
 * @param    p_cmd_args  Pointer to structure that will be filled during parse operation.
 *
 * @return   Clock command parse status:
 *               - MB_CMD_PARSE_STATUS_SUCCESS
 *               - MB_CMD_PARSE_STATUS_EMPTY
 *               - MB_CMD_PARSE_STATUS_INVALID_ARG
 *               - MB_CMD_PARSE_STATUS_HELP
 *
 * @note
 *******************************************************************************************************/
static MB_CMD_PARSE_STATUS MbCmd_CmdArgParse(CPU_INT16U  argc,
                                               CPU_CHAR    *p_argv[],
                                               MB_CMD_ARG *p_cmd_args)
{
  MB_CMD_PARSE_STATUS status;

  if (argc == 1) {
    status = MB_CMD_PARSE_STATUS_EMPTY;
    goto exit;
  }

exit_ok:
  status = MB_CMD_PARSE_STATUS_SUCCESS;

exit:

  return status;
  
}

/****************************************************************************************************//**
 *                                           MbCmd_OutputCmdTbl()
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
static CPU_INT16S MbCmd_OutputCmdTbl(SHELL_CMD       *p_cmd_tbl,
                                      SHELL_OUT_FNCT  out_fnct,
                                      SHELL_CMD_PARAM *p_cmd_param)
{
  SHELL_CMD  *p_shell_cmd;
  CPU_INT16S ret_val;
  CPU_INT16S acc_ret_val;

  ret_val = MbCmd_OutputMsg(MB_CMD_OUTPUT_CMD_LIST,
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
    ret_val = MbCmd_OutputMsg(p_shell_cmd->Name,
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

/****************************************************************************************************//**
 *                                           MbCmd_OutputError()
 *
 * @brief    Outputs error message.
 *
 * @param    p_error         Pointer to a string describing the error.
 *
 * @param    out_fnct        Callback to a respond to the requester.
 *
 * @param    p_cmd_param     Pointer to additional information to pass to the command.
 *
 * @return   The number of positive data octets transmitted, if NO error(s).
 *               - SHELL_OUT_RTN_CODE_CONN_CLOSED, if implemented connection closed.
 *               - SHELL_OUT_ERR, otherwise.
 *******************************************************************************************************/
static CPU_INT16S MbCmd_OutputError(CPU_CHAR        *p_error,
                                     SHELL_OUT_FNCT  out_fnct,
                                     SHELL_CMD_PARAM *p_cmd_param)
{
  CPU_INT16S ret_val;
  CPU_INT16S byte_out_cnt = 0;

  ret_val = MbCmd_OutputMsg(MB_CMD_OUTPUT_ERR,
                             DEF_YES,
                             DEF_NO,
                             DEF_NO,
                             out_fnct,
                             p_cmd_param);

  MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);

  ret_val = MbCmd_OutputMsg(p_error,
                             DEF_NO,
                             DEF_YES,
                             DEF_NO,
                             out_fnct,
                             p_cmd_param);

  MB_CMD_OUT_MSG_CHK(ret_val, byte_out_cnt, exit_fail);

  ret_val = byte_out_cnt;

exit_fail:
  return (ret_val);
}

/****************************************************************************************************//**
 *                                           MbCmd_OutputMsg()
 *
 * @brief    Format and output a message.
 *
 * @param    p_msg           Pointer of char on the string to format and output.
 *
 * @param    new_line_start  If DEF_YES, will add a new line character at the start.
 *
 * @param    new_line_end    If DEF_YES, will add a new line character at the end.
 *
 * @param    tab_start       If DEF_YES, will add a tab character at the start.
 *
 * @param    out_fnct        Callback to a respond to the requester.
 *
 * @param    p_cmd_param     Pointer to additional information to pass to the command.
 *
 * @return   The number of positive data octets transmitted, if NO error(s).
 *               - SHELL_OUT_RTN_CODE_CONN_CLOSED, if implemented connection closed.
 *               - SHELL_OUT_ERR, otherwise.
 *******************************************************************************************************/
static CPU_INT16S MbCmd_OutputMsg(const CPU_CHAR  *p_msg,
                                   CPU_BOOLEAN     new_line_start,
                                   CPU_BOOLEAN     new_line_end,
                                   CPU_BOOLEAN     tab_start,
                                   SHELL_OUT_FNCT  out_fnct,
                                   SHELL_CMD_PARAM *p_cmd_param)
{
  CPU_INT16U output_len;
  CPU_INT16S output;
  CPU_INT16S byte_out_cnt = 0;

  if (new_line_start == DEF_YES) {
    output = out_fnct((CPU_CHAR *)STR_NEW_LINE,
                      STR_NEW_LINE_LEN,
                      p_cmd_param->pout_opt);

    MB_CMD_OUT_MSG_CHK(output, byte_out_cnt, exit_fail);
  }

  if (tab_start == DEF_YES) {
    output = out_fnct((CPU_CHAR *)MB_CMD_OUTPUT_TABULATION,
                      1,
                      p_cmd_param->pout_opt);

    MB_CMD_OUT_MSG_CHK(output, byte_out_cnt, exit_fail);
  }

  output_len = (CPU_INT16U)Str_Len(p_msg);
  output = out_fnct((CPU_CHAR *)p_msg,
                    output_len,
                    p_cmd_param->pout_opt);

  MB_CMD_OUT_MSG_CHK(output, byte_out_cnt, exit_fail);

  if (new_line_end == DEF_YES) {
    output = out_fnct((CPU_CHAR *)STR_NEW_LINE,
                      STR_NEW_LINE_LEN,
                      p_cmd_param->pout_opt);

    MB_CMD_OUT_MSG_CHK(output, byte_out_cnt, exit_fail);
  }

  output = byte_out_cnt;

exit_fail:
  return (output);
}

