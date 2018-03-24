/* ddcutil_c_api.h
 *
 * Public C APi for ddcutil. .
 *
 * <copyright>
 * Copyright (C) 2014-2018 Sanford Rockowitz <rockowitz@minsoft.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * </endcopyright>
 */

/** \file ddcutil_c_api.h
 *  Public C API for ddcutil
 */

#ifndef DDCUTIL_C_API_H_
#define DDCUTIL_C_API_H_

/** \cond */
#include <stdbool.h>
#include <stdio.h>
/** \endcond */

#ifdef __cplusplus
extern "C" {
#endif

#include "ddcutil_types.h"


/** @file ddcutil_c_api.h
 *  @brief ddcutil public C API
 *
 *  Function names in the public C API begin with "ddca_"\n
 *  Status codes begin with "DDCRC_".
 *  Typedefs, other constants, etc. begin with "DDCA_".
 */


/* Note on "report" functions.
 *
 * Various functions with "report" in the name, e.g. ddca_report_display_ref(),
 * ddca_report_display_info_list(), write formatted reports to (normally) the
 * terminal. Sometimes, these are intended to display data structures for
 * debugging.  Other times, they are used to format output for the ddcutil
 * command line program.
 *
 * The operation of these functions can be tweaked in two ways.
 * - The "depth" parameter is a logical indentation depth.  This allows
 *   reports that invoke other reports to indent the subreports
 *   sensibly.  At the level of the ddcutil_c_api(), one unit of
 *   logical indentation depth translates to 3 spaces.
 * - The destination of reports is normally the STDOUT device.  This can
 *   be changed by calling set_fout().
 */


//
// Library build information
//

/**
 * Returns the ddcutil version as a struct of 3 8 bit integers.
 *
 * @return version numbers
 */
DDCA_Ddcutil_Version_Spec ddca_ddcutil_version(void);       // ddcutil version

/**
 * Returns the ddcutil version as a string in the form "major.minor.micro".
 *
 * @return version string.
 */
const char * ddca_ddcutil_version_string(void);


// Bit ids for ddca_get_build_options() - how to make connection in doxygen?
/** Build option flags
 *
 *  The enum values are defined as 1,2,4 etc so that they can be or'd.
 */
typedef enum {
   /** @brief ddcutil was built with support for AMD Display Library connected monitors */
   DDCA_BUILT_WITH_ADL     = 0x01,
   /** @brief ddcutil was built with support for USB connected monitors */
   DDCA_BUILT_WITH_USB     = 0x02,
  /** @brief ddcutil was built with support for failure simulation */
   DDCA_BUILT_WITH_FAILSIM = 0x04
} DDCA_Build_Option_Flags;


#ifdef ALT
 * Defined Bits
 * <table>
 * <tr><td>#DDCA_BUILT_WITH_ADL</td><td>built with ADL support</td>
 * <tr><td>#DDCA_BUILT_WITH_USB</td><td>built with USB support
 * <tr><td>#DDCA_BUILT_WITH_FAILSIM</td><td> built with failure simulation
 * </table>
#endif

/** Queries the options with which the **ddcutil** library was built.
 *
 * @return  flags byte
 *
 * | Defined Bits: | |
 * |:-------| :--------------
 * |#DDCA_BUILT_WITH_ADL  | built with ADL support
 * |#DDCA_BUILT_WITH_USB  | built with USB support
 * |#DDCA_BUILT_WITH_FAILSIM | built with failure simulation
 *
 */
DDCA_Build_Option_Flags ddca_build_options(void);


//
// Status Codes
//

/** Returns the symbolic name for a ddcutil status code
 *
 * @param[in] status_code numeric status code
 * @return    symbolic name, e.g. EBUSY, DDCRC_INVALID_DATA
 * @retval    NULL if unrecognized code
 *
 * @remark
 * The returned value is a pointer into internal persistent
 * data structures and should not be free'd by the caller.
 *  */
char * ddca_rc_name(DDCA_Status status_code);

/** Returns a description of a ddcutil status code
 *
 * @param[in] status_code numeric status code
 * @return    explanation of status code, e.g. "device or resource busy"
 * @retval    "unknown status code" if unrecognized code
 *
 * @remark
 * The returned value is a pointer into internal persistent
 * data structures and should not be free'd by the caller.
 */
char * ddca_rc_desc(DDCA_Status status_code);


//
// MCCS Version Id
//

/** \deprecated
 *  Returns the symbolic name of a #DDCA_MCCS_Version_Id,
 *  e.g. "DDCA_MCCS_V20."
 *
 *  @param[in]  version_id  version id value
 *  @return symbolic name (do not free)
 */
char *
ddca_mccs_version_id_name(
      DDCA_MCCS_Version_Id  version_id) __attribute__ ((deprecated));

/** \deprecated
 *  Returns the descriptive name of a #DDCA_MCCS_Version_Id,
 *  e.g. "2.0".
 *
 *  @param[in]  version_id  version id value
 *  @return descriptive name (do not free)
 *
 *  @remark added to replace ddca_mccs_version_id_desc() during 0.9
 *  development, but then use of DDCA_MCCS_Version_Id deprecated
 */
char *
__attribute__ ((deprecated))
ddca_mccs_version_id_desc(
      DDCA_MCCS_Version_Id  version_id) ;


//
// Global Settings
//

/***
I2C is an inherently unreliable protocol.  The application is responsible for
retry management.
The maximum number of retries can be tuned.
There are 3 retry contexts:
- An I2C write followed by a read.  Most DDC operations are of this form.
- An I2C write without a subsequent read.  DDC operations to set a VCP feature
  value are in this category.
- Some DDC operations, such as reading the capabilities string, reading table
  feature and writing table features require multiple
  write/read exchanges.  These multi-part exchanges have a separate retry count
  for the entire operation.
*/
///@{
/** Gets the upper limit on a max tries value that can be set.
 *
 * @return maximum max tries value allowed on set_max_tries()
 */
int ddca_max_max_tries(void);

/** Gets the maximum number of I2C retries for the specified operation type.
 * @param[in]  retry_type   I2C operation type
 * @return maximum number of retries
 *
 * @remark
 * This setting is global, not thread-specific.
 */
int ddca_get_max_tries(
       DDCA_Retry_Type retry_type);

/** Sets the maximum number of I2C retries for the specified operation type
 * @param[in] retry_type    I2C operation type
 * @param[in] max_tries     maximum count to set
 * @retval   -EINVAL        max_tries < 1 or > #ddca_get_max_tries()
 *
 * @remark
 * This setting is global, not thread-specific.
 */
DDCA_Status
ddca_set_max_tries(
      DDCA_Retry_Type retry_type,
      int             max_tries);
///@}

/** Controls whether VCP values are read after being set.
 *
 * \param[in] onoff true/false
 * \return  prior value
 *
 * \remark This setting is thread-specific.
 */
bool ddca_enable_verify(bool onoff);

/** Query whether VCP values are read after being set.
 * \retval true values are verified after being set
 * \retval false values are not verified
 *
 * \remark This setting is thread-specific.
 */
bool ddca_is_verify_enabled(void);


//
// Output Redirection
//

/** Redirects output on the current thread that normally would go to **stdout**  */
void ddca_set_fout(
        FILE * fout);   /**< where to write normal messages, if NULL, suppress output */

/** Redirects output on the current thread that normally goes to **stdout** back to **stdout** */
void ddca_set_fout_to_default(void);

/** Redirects output on the current thread that normally would go to **stderr**  */
void ddca_set_ferr(
        FILE * ferr);   /**< where to write error messages, If NULL, suppress output */

/** Redirects output on the current thread that normally goes to **stderr** back to **stderr** */
void ddca_set_ferr_to_default(void);


//
// Convenience functions for capturing output by redirecting
// to an in-memory buffer.
//

/** Capture option flags
 *
 *  The enum values are defined as 1,2,4 etc so that they can be or'd.
 *
 *  @since 0.9.0
 */
typedef enum {
   DDCA_CAPTURE_NOOPTS     = 0,     ///< @brief no options specified
   DDCA_CAPTURE_STDERR     = 1      ///< @brief capture **stderr** as well as **stdout**
} DDCA_Capture_Option_Flags;

/** Begins capture of **stdout** and optionally **stderr** output on the
 *  current thread to a thread-specific in-memory buffer.
 *
 *  @note  If output is already being captured, this function has no effect.
 *  @since 0.9.0
 */
void ddca_start_capture(DDCA_Capture_Option_Flags flags);

/** Ends capture of **stdout** output and returns the contents of the
 *  in-memory buffer.
 *
 *  Upon termination, normal thread output is directed to **stdout**.
 *  If error output was also being captured, error output is redirected
 *  to **stderr**.
 *
 *  @return captured output as a string.
 *
 *  @note  Writes messages to actual **stderr** in case of error.
 *  @since 0.9.0
 */
char * ddca_end_capture(void);


//
// Message Control
//

/** Gets the current output level for the current thread */
DDCA_Output_Level                   /**< current output level */
ddca_get_output_level(void);

/** Sets the output level for the current thread  */
DDCA_Output_Level                  /**< old output level */
ddca_set_output_level(
      DDCA_Output_Level newval);   /**< new output level */

/** Gets the name of an output level
 *  @param[in]  val  output level id
 *  @return     output level name (do not free)
 */
char * ddca_output_level_name(
          DDCA_Output_Level val);   /**< output level id */


/** Controls whether messages describing DDC protocol errors are output
 *  @param[in] onoff    if true, errors will be issued
 *  @return    prior value
 *
 *  This setting is global to all threads.
 */
bool ddca_enable_report_ddc_errors(bool onoff);

/** Checks whether messages describing DDC protocol errors are output.
 *
 *  This setting is global to all threads.
 */
bool ddca_is_report_ddc_errors_enabled(void);


//
// Statistics
//
// Statistics are global to all threads.
//

/** Resets all **ddcutil** statistics */
void ddca_reset_stats(void);

/** Show execution statistics.
 *
 *  \param[in] stats  bitflags of statistics types to show
 *  \param[in] depth  logical indentation depth
 */
void ddca_show_stats(DDCA_Stats_Type stats, int depth);

// TODO: Add functions to get stats


//
// Display Descriptions
//

/** @deprecated use #ddca_get_display_info_list2()
 * Gets a list of the detected displays.
 *
 *  Displays that do not support DDC are not included.
 *
 *  @return list of display summaries
 */
// __attribute__ ((deprecated))
DDCA_Display_Info_List *
ddca_get_display_info_list(void);

/** Gets a list of the detected displays.
 *
 *  @param[in]  include_invalid_displays if true, displays that do not support DDC are included
 *  @param[out] dlist_loc where to return pointer to #DDCA_Display_Info_List
 *  @retval     0  always succeeds
 */
DDCA_Status
ddca_get_display_info_list2(
      bool                      include_invalid_displays,
      DDCA_Display_Info_List**  dlist_loc);

/** Frees a list of detected displays.
 *
 *  This function understands which fields in the list
 *  point to permanently allocated data structures and should
 *  not be freed.
 *
 *  \param[in] dlist pointer to #DDCA_Display_Info_List
 */
void ddca_free_display_info_list(DDCA_Display_Info_List * dlist);

/** Presents a report on a single display.
 *  The report is written to the current FOUT device for the current thread.
 *
 *  @param[in]  dinfo  pointer to a DDCA_Display_Info struct
 *  @param[in]  depth  logical indentation depth
 *
 *  @remark
 *  For a report intended for users, apply #ddca_report_display_by_dref()
 *  to **dinfo->dref**.
 */
void
ddca_dbgrpt_display_info(
      DDCA_Display_Info * dinfo,
      int                 depth);

/** Reports on all displays in a list of displays.
 *  The report is written to the current FOUT device for the current thread.
 *
 *  @param[in]  dlist  pointer to a DDCA_Display_Info_List
 *  @param[in]  depth  logical indentation depth
 */
void
ddca_dbgrpt_display_info_list(
      DDCA_Display_Info_List * dlist,
      int                      depth);

/** Reports on all active displays.
 *  This function hooks into the code used by command "ddcutil detect"
 *
 *  @param[in] include_invalid_displays if true, report displays that don't support DDC
 *  @param[in] depth  logical indentation depth
 *  @return    number of MCCS capable displays
 */
int
ddca_report_active_displays(
      bool include_invalid_displays,
      int  depth);


//
// Display Identifier
//

/** Creates a display identifier using the display number assigned by ddcutil
 * @param[in]  dispno  display number
 * @param[out] did_loc    where to return display identifier handle
 * @retval     0
 *
 * \ingroup api_display_spec
 * */
DDCA_Status
ddca_create_dispno_display_identifier(
      int                      dispno,
      DDCA_Display_Identifier* did_loc);

/** Creates a display identifier using an I2C bus number
 * @param[in]  busno  I2C bus number
 * @param[out] did_loc   where to return display identifier handle
 * @retval     0
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_create_busno_display_identifier(
      int                      busno,
      DDCA_Display_Identifier* did_loc);

/** Creates a display identifier using an ADL (adapter index, display index) pair
 * @param[in]  iAdapterIndex ADL adapter index
 * @param[in]  iDisplayIndex ADL display index
 * @param[out] did_loc          where to return display identifier handle
 * @return     status code
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_create_adlno_display_identifier(
      int                      iAdapterIndex,
      int                      iDisplayIndex,
      DDCA_Display_Identifier* did_loc);

/** Creates a display identifier using some combination of the manufacturer id,
 * model name string and serial number string.  At least 1 of the 3 must be specified.
 * @param[in]  mfg_id  3 letter manufacturer id
 * @param[in]  model   model name string
 * @param[in]  sn     serial number string
 * @param[out] did_loc   where to return display identifier handle
 * @retval 0       success
 * @retval -EINVAL no argument specified, or argument too long
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_create_mfg_model_sn_display_identifier(
      const char *             mfg_id,
      const char *             model,
      const char *             sn,
      DDCA_Display_Identifier* did_loc);

/** Creates a display identifier using a 128 byte EDID
 * @param[in]   edid  pointer to 128 byte EDID
 * @param[out]  did_loc  where to return display identifier handle
 * @retval  0       success
 * @retval  -EINVAL edid==NULL
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_create_edid_display_identifier(
      const uint8_t*            edid,
      DDCA_Display_Identifier * did_loc);      // 128 byte edid

/** Creates a display identifier using a USB bus number and device number
 * @param[in]  bus    USB bus number
 * @param[in]  device USB device number
 * @param[out] did_loc   where to return display identifier handle
 * @retval 0 success
 *
 *  \ingroup api_display_spec
 */
DDCA_Status
ddca_create_usb_display_identifier(
      int                      bus,
      int                      device,
      DDCA_Display_Identifier* did_loc);

/** Creates a display identifier using a /dev/usb/hiddev device number
 * @param[in] hiddev_devno hiddev device number
 * @param[out] did_loc   where to return display identifier handle
 * @retval 0  success
 *
 *  \ingroup api_display_spec
 */
DDCA_Status
ddca_create_usb_hiddev_display_identifier(
      int                      hiddev_devno,
      DDCA_Display_Identifier* did_loc);


/** Release the memory of a display identifier
 * @param[in] did  display identifier, may be NULL
 * @retval 0          success
 * @retval DDCRC_ARG  invalid display identifier
 *
 * @remark
 * Does nothing and returns 0 if **did** is NULL.
 */
DDCA_Status
ddca_free_display_identifier(
      DDCA_Display_Identifier did);

/** Returns a string representation of a display identifier.
 *
 *  The string is valid until the display identifier is freed.
 *
 *  \param[in]  did    display identifier
 *  \return     string representation of display identifier, NULL if invalid
 *
 *  \ingroup api_display_spec
 */
char *
ddca_did_repr(
      DDCA_Display_Identifier did);


//
// Display Reference
//

/** Gets a display reference for a display identifier.
 *  Normally, this is a permanently allocated #DDCA_Display_Ref
 *  created by monitor detection and does not need to be freed.
 *  Use #ddca_free_display_ref() to safely free.
 * @param[in]  did      display identifier
 * @param[out] dref_loc where to return display reference
 * @retval     0 success
 * @retval     -EINVAL  did is not a valid display identifier handle
 * @retval     DDCRC_INVALID_DISPLAY display not found
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_create_display_ref(
      DDCA_Display_Identifier did,
      DDCA_Display_Ref*       dref_loc);

/** Frees a display reference.
 *
 * Use this function to safely release a #DDCA_Display_Ref.
 * If the display reference was dynamically created, it is freed.
 * If the display references was permanently allocated (normal case), does nothing.
 * @param[in] dref  display reference to free
 * @return status code
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_free_display_ref(
      DDCA_Display_Ref dref);

/** Returns a string representation of a display reference
 *
 *  The returned value is valid until the next call to this function on
 *  the current thread.
 *
 *  @param[in]   dref display reference
 *  @return      string representation of display reference, NULL if invalid
 */
char *
ddca_dref_repr(
      DDCA_Display_Ref dref);

/** Writes a report on the specified display reference to the current FOUT device
 * @param[in] dref   display reference
 * @param[in] depth  logical indentation depth
 *
 * \ingroup api_display_spec
 */
void
ddca_dbgrpt_display_ref(
      DDCA_Display_Ref dref,
      int              depth);


//
// Display Handle
//

/** Open a display
 * @param[in]  ddca_dref    display reference for display to open
 * @param[out] ddca_dh_loc  where to return display handle
 * @return     status code
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_open_display(
      DDCA_Display_Ref      ddca_dref,
      DDCA_Display_Handle * ddca_dh_loc);

/** Close an open display
 * @param[in]  ddca_dh   display handle
 * @return     DDCA status code
 *
 * \ingroup api_display_spec
 */
DDCA_Status
ddca_close_display(
      DDCA_Display_Handle   ddca_dh);

/** Returns a string representation of a display handle.
 *  The string is valid until until the handle is closed.
 *
 * @param[in] ddca_dh  display handle
 * @return string  representation of display handle, NULL if
 *                 argument is NULL or not a display handle
 *
 *  \ingroup api_display_spec
 */
char *
ddca_dh_repr(
      DDCA_Display_Handle   ddca_dh);


//
// Monitor Capabilities
//

/** Retrieves the capabilities string for a monitor.
 *
 *  @param[in]  ddca_dh     display handle
 *  @param[out] caps_loc    address at which to return pointer to capabilities string.
 *  @return     status code
 *
 *  It is the responsibility of the caller to free the returned string.
 */
DDCA_Status
ddca_get_capabilities_string(
      DDCA_Display_Handle     ddca_dh,
      char**                  caps_loc);

/** Parse the capabilities string.
 *
 *  @param[in] capabilities_string      unparsed capabilities string
 *  @param[out] parsed_capabilities_loc address at which to return pointer to newly allocated
 *                                      #DDCA_Capabilities struct
 *  @return     status code
 *
 *  It is the responsibility of the caller to free the returned struct
 *  using ddca_free_parsed_capabilities().
 *
 *  This function currently parses the VCP feature codes and MCCS version.
 *  It could be extended to parse additional information such as cmds if necessary.
 */
DDCA_Status
ddca_parse_capabilities_string(
      char *                   capabilities_string,
      DDCA_Capabilities **     parsed_capabilities_loc);

/** Frees a DDCA_Capabilities struct
 *
 *  @param[in]  parsed_capabilities  pointer to struct to free.
 *                                   Does nothing if NULL.
 */
void
ddca_free_parsed_capabilities(
      DDCA_Capabilities *      parsed_capabilities);

/** Reports the contents of a DDCA_Capabilities struct.
 *
 *  The report is written to the current FOUT location.
 *
 *  @param[in]  parsed_capabilities  pointer to #DDCA_Capabilities struct
 *  @param[in]  depth  logical       indentation depth
 */
void
ddca_report_parsed_capabilities(
      DDCA_Capabilities *      parsed_capabilities,
      int                      depth);


//
// VCP Feature Information, Monitor Independent
//

/** \deprecated Use #ddca_get_feature_flags_by_vspec()
 *
 * Gets information for a VCP feature.
 *
 * VCP characteristics (C vs NC, RW vs RO, etc) can vary by MCCS version.
 *
 * @param[in]  feature_code    VCP feature code
 * @param[in]  mccs_version_id MCCS version id, may be DDCA_VCP_VANY??
 * @param[out] info_loc        where to return Version_Feature_Info struct
 * @return     status code
 */
DDCA_Status
ddca_get_feature_info_by_vcp_version(
      DDCA_Vcp_Feature_Code         feature_code,
      DDCA_MCCS_Version_Id          mccs_version_id,
      DDCA_Version_Feature_Info**   info_loc) __attribute__ ((deprecated));


/** \deprecated  Use #ddca_get_feature_flags_by_vspec() */
DDCA_Status
ddca_get_simplified_feature_info(
      DDCA_Vcp_Feature_Code         feature_code,
      DDCA_MCCS_Version_Spec        vspec,
      DDCA_Simplified_Version_Feature_Info *  info)     // caller buffer to fill in,
      __attribute__ ((deprecated));

/**
 * Gets information for a VCP feature.
 *
 * VCP characteristics (C vs NC, RW vs RO, etc) can vary by MCCS version.
 *
 * @param[in]  feature_code     VCP feature code
 * @param[in]  vspec            MCCS version id
 * @param[out] p_feature_flags  address of flag field to fill in
 * @return     status code
 */
DDCA_Status
ddca_get_feature_flags_by_vspec(
      DDCA_Vcp_Feature_Code         feature_code,
      DDCA_MCCS_Version_Spec        vspec,
      DDCA_Feature_Flags *          p_feature_flags);


/** \deprecated Use ddca_get_feature_flags_by_vspec() */
DDCA_Status
ddca_get_feature_flags_by_version_id(
      DDCA_Vcp_Feature_Code         feature_code,
      DDCA_MCCS_Version_Id          mccs_version_id,
      DDCA_Feature_Flags *          feature_flags) __attribute__ ((deprecated));

/** \deprecated Use #ddca_get_feature_flags_by_vspec() */
DDCA_Status
ddca_get_feature_info_by_display(
      DDCA_Display_Handle           ddca_dh,
      DDCA_Vcp_Feature_Code         feature_code,
      DDCA_Version_Feature_Info **  info_loc) __attribute__ ((deprecated));


DDCA_Status
ddca_get_simplified_feature_info_by_display(
      DDCA_Display_Handle           ddca_dh,
      DDCA_Vcp_Feature_Code         feature_code,
      DDCA_Simplified_Version_Feature_Info *  info);     // caller buffer to fill in,




/** Gets the VCP feature name.  If different MCCS versions use different names
 *  for the feature, this function makes a best guess.
 *
 * @param[in]  feature_code feature code
 * @return     pointer to feature name (do not free), NULL if unknown feature code
 */
char *
ddca_get_feature_name(DDCA_Vcp_Feature_Code feature_code);


/** Gets the VCP feature name, which may vary by MCCS version.
 *
 * @param[in]  feature_code  feature code
 * @param[in]  vspec         MCCS version
 * @return     pointer to feature name (do not free), NULL if unknown feature code
 */
char *
ddca_feature_name_by_vspec(
      DDCA_Vcp_Feature_Code  feature_code,
      DDCA_MCCS_Version_Spec vspec);

/** \deprecated */
char *
ddca_feature_name_by_version_id(
      DDCA_Vcp_Feature_Code  feature_code,
      DDCA_MCCS_Version_Id   mccs_version_id) __attribute__ ((deprecated));


/** Gets the value id/name table of the allowed values for a simple NC feature.
 *
 * @param[in]  feature_code      VCP feature code
 * @param[in]  vspec             MCCS version
 * @param[out] value_table_loc   where to return pointer to array of DDCA_Feature_Value_Entry
 * @return     status code
 * @retval     0                       success
 * @retval     DDCRC_UNKNOWN_FEATURE   unrecognized feature code
 * @retval     DDCRC_INVALID_OPERATION feature not simple NC
 */
DDCA_Status
ddca_get_simple_sl_value_table_by_vspec(
      DDCA_Vcp_Feature_Code      feature_code,
      DDCA_MCCS_Version_Spec     vspec,
      DDCA_Feature_Value_Entry** value_table_loc);

/** \deprecated Use #ddca_get_simple_sl_value_table_by_vspec() */
DDCA_Status
ddca_get_simple_sl_value_table(
      DDCA_Vcp_Feature_Code       feature_code,
      DDCA_MCCS_Version_Id        mccs_version_id,
      DDCA_Feature_Value_Table *  value_table_loc)   // DDCA_Feature_Value_Entry **
     __attribute__ ((deprecated));

// TODO: Implement
/** Gets the value id/name table of the allowed values for a simple NC feature.
 *
 * @param[in]  vspec             MCCS version
 * @param[in]  feature_code      VCP feature code
 * @param[in]  feature_value     single byte feature value
 * @param[out] feature_name_loc  where to return feature name
 * @return     status code
 *
 * @remark
 * If the feature value cannot be found in the lookup table for
 * the specified MCCS version, tables for later versions, if they
 * exist, are checked as well.
 */
DDCA_Status
ddca_get_simple_nc_feature_value_name_by_vspec(
      DDCA_MCCS_Version_Spec vspec,
      DDCA_Vcp_Feature_Code  feature_code,
      uint8_t                feature_value,
      char**                 feature_name_loc);


DDCA_Status
ddca_get_simple_nc_feature_value_name(
      DDCA_Display_Handle    ddca_dh,    // needed because value lookup mccs version dependent
      DDCA_Vcp_Feature_Code  feature_code,
      uint8_t                feature_value,
      char**                 feature_name_loc);


/** \deprecated */
DDCA_Status
ddca_free_feature_info(
      DDCA_Version_Feature_Info * info) __attribute__ ((deprecated));


//
// VCP Feature Information, Monitor Dependent
//

#ifdef UNIMPLEMENTED

// Unimplemented
// alt: can check status code for ddca_get_feature_info_by_display()
DDCA_Status ddct_is_feature_supported(
      DDCA_Display_Handle    dh,
      DDCA_Vcp_Feature_Code  feature_code,
      bool *                 answer_loc);

#endif


//
//  Miscellaneous Monitor Specific Functions
//

/** Gets the MCCS version of a monitor.
 *
 *  @param[in]    ddca_dh  display handle
 *  @param[out]   p_vspec  where to return version spec
 */
DDCA_Status
ddca_get_mccs_version(
      DDCA_Display_Handle     ddca_dh,
      DDCA_MCCS_Version_Spec* p_vspec);

/** \deprecated */
DDCA_Status
ddca_get_mccs_version_id(
      DDCA_Display_Handle     ddca_dh,
      DDCA_MCCS_Version_Id*   p_version_id) __attribute__ ((deprecated));


#ifdef UNIMPLEMENTED
// Use ddca_get_edid_by_dref() instead
// n. edid_buffer must be >= 128 bytes

DDCA_Status
ddca_get_edid(DDCA_Display_Handle * dh, uint8_t* edid_buffer);
#endif

DDCA_Status
ddca_get_edid_by_dref(
      DDCA_Display_Ref ddca_dref,
      uint8_t **       pbytes_loc);   // pointer into ddcutil data structures, do not free


/** Shows information about a display, specified by a #Display_Ref
 *
 * Output is written using report functions
 *
 * \param dref   pointer to display reference
 * \param depth  logical indentation depth
 * \retval DDCRC_ARG invalid display ref
 * \retval 0         success
 *
 * \remark
 * The detail level shown is controlled by the output level setting
 * for the current thread.
 */
DDCA_Status
ddca_report_display_by_dref(DDCA_Display_Ref dref, int depth);


//
// Feature Lists
//
// Specifies a collection of VCP features as a 256 bit array of flags.
//

/** Given a feature set id, returns a #DDCA_Feature_List specifying all the
 *  feature codes in the set.
 *
 *  @param[in]  feature_set_id
 *  @param[in]  vcp_version
 *  @param[in]  include_table_features if true, Table type features are included
 *  @param[out] points to feature list to be filled in
 */
DDCA_Status
ddca_get_feature_list(
      DDCA_Feature_Subset_Id  feature_set_id,
      DDCA_MCCS_Version_Spec  vcp_version,
      bool                    include_table_features,
      DDCA_Feature_List*      p_feature_list);


/** Empties a #DDCA_Feature_List
 *
 *  @param[in]  vcplist pointer to feature list
 */
void ddca_feature_list_clear(
      DDCA_Feature_List* vcplist);

/** Adds a feature code to a #DDCA_Feature_List
 *
 *  @param[in]  vcplist   pointer to feature list
 *  @param[in]  vcp_code  VCP feature code
 */
void
ddca_feature_list_add(
      DDCA_Feature_List* vcplist,
      uint8_t vcp_code);

/** Tests if a #DDCA_Feature_List contains a VCP feature code
 *
 *  @param[in]  vcplist   pointer to feature list
 *  @param[in]  vcp_code  VCP feature code
 *  @return     true/false
 */
bool
ddca_feature_list_contains(
      DDCA_Feature_List* vcplist,
      uint8_t vcp_code);

/** Creates a union of 2 feature lists.
 *
 *  @param[in] vcplist1   pointer to first feature list
 *  @param[in] vcplist2   pointer to second feature list
 *  @return feature list in which a feature is set if it is in either
 *          or the 2 input feature lists
 *
 *  @remark
 *  The input feature lists are not modified.
 */
DDCA_Feature_List
ddca_feature_list_union(
      DDCA_Feature_List* vcplist1,
      DDCA_Feature_List * vcplist2);

/** Returns a feature list consisting of all the features in the
 *  first list that are not in the second.
 *
 *  @param[in] vcplist1   pointer to first feature list
 *  @param[in] vcplist2   pointer to second feature list
 *  @return feature list in which a feature is set if it is in **vcplist1** but
 *          not **vcplist2**
 *
 *  @remark
 *  The input feature lists are not modified.
 */
DDCA_Feature_List
ddca_feature_list_subtract(
      DDCA_Feature_List* vcplist1,
      DDCA_Feature_List* vcplist2);


/** Returns a feature list consisting of all the features in the
 *  first list that are not in the second.
 *
 *  @param[in]  vcplist   pointer to feature list
 *  @param[out] p_codect  address where to return count of feature codes
 *  @param[out] vcp_codes address of 256 byte buffer to receive codes
 */
void ddca_feature_list_to_codes(
      DDCA_Feature_List* vcplist,
      int*               p_codect,
      uint8_t            vcp_codes[256]);

/*
 * The API for getting and setting VCP values is doubly specified,
 * with both functions specific to Non-Table and Table values,
 * and more generic functions that can handle values of any type.
 *
 * As a practical matter, Table type features have not been observed
 * on any monitors (as of 3/2018), and applications can probably
 * safely be implemented using only the Non-Table APIs.
 *
 * Note that the functions for #DDCA_Any_Vap_Value replace those
 * that previously existed for #DDCA_Single_Vcp_Value.
 */

//
// Free VCP Feature Value
//
// Note there is no function to free a #Non_Table_Vcp_Value, since
// this is a fixed size struct always allocated by the caller.
//

/** Frees a #DDCA_Table_Vcp_Value instance.
 *
 *  @param[in] table_value
 *
 *  @remark
 *  Was previously named **ddca_free_table_value_response().
 */
void
ddca_free_table_vcp_value(
      DDCA_Table_Vcp_Value * table_value);

/** Frees a #DDCA_Any_Vcp_Value instance.
 *
 *  @param[in] valrec  pointer to #DDCA_Any_Vcp_Value instance
 */
void
ddca_free_any_vcp_value(
      DDCA_Any_Vcp_Value * valrec);


/** Produces a debugging report of a #DDCA_Any_Vcp_Value instance.
 *  The report is written to the current FOUT device.
 *  @param  valrec  instance to report
 *  @param  depth   logical indentation depth
 */
void
dbgrpt_any_vcp_value(
      DDCA_Any_Vcp_Value * valrec,
      int                  depth);


//
// Get VCP Feature Value
//

/** Gets the value of a non-table VCP feature.
 *
 * @param[in]  ddca_dh       display handle
 * @param[in]  feature_code  VCP feature code
 * @param[out] valrec        pointer to response buffer provided by the caller,
 *                           which will be filled in
 * @return status code
 */
DDCA_Status
ddca_get_non_table_vcp_value(
       DDCA_Display_Handle        ddca_dh,
       DDCA_Vcp_Feature_Code      feature_code,
       DDCA_Non_Table_Vcp_Value*  valrec);


/** Gets the value of a table VCP feature.
 *
 * @param[in]  ddca_dh         display handle
 * @param[in]  feature_code    VCP feature code
 * @param[out] table_value_loc address at which to return the value
 * @return status code
 *
 * @note
 * Implemented, but untested
 */
DDCA_Status
ddca_get_table_vcp_value(
       DDCA_Display_Handle     ddca_dh,
       DDCA_Vcp_Feature_Code   feature_code,
       DDCA_Table_Vcp_Value **     table_value_loc);


/** Gets the value of a VCP feature of any type.
 *
 * @param[in]  ddca_dh       display handle
 * @param[in]  feature_code  VCP feature code
 * @param[in]  value_type    value type
 * @param[out] valrec_loc    address at which to return a pointer to a newly
 *                           allocated #DDCA_Any_Vcp_Value
 * @return status code
 *
 * @remark
 * Parm **value_type** is needed only manufacturer-specific features.
 * If set to #DDCA_UNSET_VCP_VALUE_TYPE_PARM, it will be ignored.
 * @remark
 * Replaces **ddca_get_any_vcp_value()
 */
DDCA_Status
ddca_get_any_vcp_value_using_explicit_type(
       DDCA_Display_Handle         ddca_dh,
       DDCA_Vcp_Feature_Code       feature_code,
       DDCA_Vcp_Value_Type_Parm    value_type,
       DDCA_Any_Vcp_Value **       valrec_loc);


/** Gets the value of a VCP feature of any type.
 *  The type is determined by using ddcutil's internal
 *  feature description table.
 *
 *  Note that this function cannot be used for manufacturer-specific
 *  feature codes (i.e. those in the range xE0..xFF), since ddcutil
 *  does not know their type information.  Nor can it be used for
 *  unrecognized feature codes.
 *
 * @param[in]  ddca_dh       display handle
 * @param[in]  feature_code  VCP feature code
 * @param[out] valrec_loc    address at which to return a pointer to a newly
 *                           allocated #DDCA_Any_Vcp_Value
 * @return status code
 *
 * @remark
 * It an error to call this function for a manufacturer-specific feature or
 * an unrecognized feature.
 */
DDCA_Status
ddca_get_any_vcp_value_using_implicit_type(
       DDCA_Display_Handle         ddca_dh,
       DDCA_Vcp_Feature_Code       feature_code,
       DDCA_Any_Vcp_Value **       valrec_loc);


/** Returns a string containing a formatted representation of the VCP value
 *  of a feature.  It is the responsibility of the caller to free this value.
 *
 *  @param[in]  ddca_dh             Display handle
 *  @param[in]  feature_code        VCP feature code
 *  @param[out] formatted_value_loc Address at which to return the formatted value
 *  @return     status code, 0 if success
 */
DDCA_Status
ddca_get_formatted_vcp_value(
       DDCA_Display_Handle     ddca_dh,
       DDCA_Vcp_Feature_Code   feature_code,
       char**                  formatted_value_loc);


/** Returns a formatted representation of a table VCP value.
 *  It is the responsibility of the caller to free the returned string.
 *
 *  @param[in]  feature_code        VCP feature code
 *  @param[in]  vspec               MCCS version
 *  @param[in]  table_value         table VCP value
 *  @param[out] formatted_value_loc address at which to return the formatted value.
 *  @return                         status code, 0 if success
 */
DDCA_Status
ddca_format_table_vcp_value(
      DDCA_Vcp_Feature_Code   feature_code,
      DDCA_MCCS_Version_Spec  vspec,
      DDCA_Table_Vcp_Value *  table_value,
      char **                 formatted_value_loc);

/** Returns a formatted representation of a non-table VCP value.
 *  It is the responsibility of the caller to free the returned string.
 *
 *  @param[in]  feature_code        VCP feature code
 *  @param[in]  vspec               MCCS version
 *  @param[in]  valrec              non-table VCP value
 *  @param[out] formatted_value_loc address at which to return the formatted value.
 *  @return                         status code, 0 if success
 */
DDCA_Status
ddca_format_non_table_vcp_value(
      DDCA_Vcp_Feature_Code   feature_code,
      DDCA_MCCS_Version_Spec  vspec,
      DDCA_Non_Table_Vcp_Value *  valrec,
      char **                 formatted_value_loc);


/** Returns a formatted representation of a VCP value of any type
 *  It is the responsibility of the caller to free the returned string.
 *
 *  @param[in]  feature_code        VCP feature code
 *  @param[in]  vspec               MCCS version
 *  @param[in]  valrec              non-table VCP value
 *  @param[out] formatted_value_loc address at which to return the formatted value.
 *  @return                         status code, 0 if success
 */
DDCA_Status
ddca_format_any_vcp_value(
      DDCA_Vcp_Feature_Code   feature_code,
      DDCA_MCCS_Version_Spec  vspec,
      DDCA_Any_Vcp_Value *    valrec,
      char **                 formatted_value_loc);


//
// Set VCP value
//

/** Sets a non-table VCP value by specifying it's high and low bytes individually.
 *  Optionally returns the values set by reading the feature code after writing.
 *
 *  \param[in]   ddca_dh             display handle
 *  \param[in]   feature_code        feature code
 *  \param[in]   hi_byte             high byte of new value
 *  \param[in]   lo_byte             low byte of new value
 *  \param[out]  p_verified_hi_byte  where to return high byte of verified value
 *  \param[out]  p_verified_lo_byte  where to return low byte of verified value
 *  \return      status code
 *
 *  \remark
 *  Either both **verified_hi_byte_loc** and **verified_lo_byte_loc** should be
 *  set, or neither. Otherwise, status code **DDCRC_ARG** is returned.
 *  \remark
 *  Verification is performed only it has been enabled (see #ddca_enable_verify()) and
 *  both **verified_hi_byte** and **verified_lo_byte** are set.
 *  \remark
 *  Verified values are returned if either the status code is either 0 (success),
 *  or **DDCRC_VERIFY**, i.e. the write succeeded but verification failed.
 */
DDCA_Status
ddca_set_non_table_vcp_value_verify(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code,
      uint8_t                  hi_byte,
      uint8_t                  lo_byte,
      uint8_t *                p_verified_hi_byte,
      uint8_t *                p_verified_lo_byte
     );


/** Sets a non-table VCP value by specifying it's high and low bytes individually.
 *
 *  \param[in]   ddca_dh             display handle
 *  \param[in]   feature_code        feature code
 *  \param[in]   hi_byte             high byte of new value
 *  \param[in]   lo_byte             low byte of new value
 *  \return      status code
 */
DDCA_Status
ddca_set_non_table_vcp_value(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code,
      uint8_t                  hi_byte,
      uint8_t                  lo_byte
     );


/** Sets a Continuous VCP value.
 *
 *  @param[in]  ddca_dh             display_handle
 *  @param[in]  feature_code        VCP feature code
 *  @param[in]  new_value           value to set (sign?)
 *  @param[out] verified_value_loc  if non-null, return verified value here
 *  @return status code
 *
 * @remark
 *  Verification is performed if **verified_value_loc** is non-NULL and
 *  verification has been enabled (see #ddca_enable_verify()).
 *  @remark
 *  If verification is performed, the value of the feature is read after being
 *  written. If the returned status code is either DDCRC_OK (0) or DDCRC_VERIFY,
 *  the verified value is returned in **verified_value_loc**.
 *  @remark
 *  This is essentially a convenience function, since a Continuous value can be
 *  set by passing its high and low bytes to #ddca_set_non_table_vcp_value_verify().
 */
DDCA_Status
ddca_set_continuous_vcp_value_verify(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code,
      uint16_t                 new_value,
      uint16_t *               verified_value_loc);


/** Sets a Continuous VCP value.
 *
 *  @param[in]  ddca_dh             display_handle
 *  @param[in]  feature_code        VCP feature code
 *  @param[in]  new_value           value to set (sign?)
 *  @return status code
 *
 * @remark
 *  This is essentially a convenience function, since a Continuous value
 *  can be set by passing its high and low bytes to #ddca_set_non_table_vcp_value().
 */
DDCA_Status
ddca_set_continuous_vcp_value(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code,
      uint16_t                 new_value);


/** @deprecated Use #ddca_set_non_table_vcp_value() */
DDCA_Status
ddca_set_simple_nc_vcp_value(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code,
      uint8_t                  new_value) __attribute__ ((deprecated));


/** Sets a table VCP value.
 *  Optionally returns the value set by reading the feature code after writing.
 *
 *  \param[in]   ddca_dh             display handle
 *  \param[in]   feature_code        feature code
 *  \param[in]   new_value           value to set
 *  \param[out]  verified_value_loc  where to return verified value
 *  \return      status code
 *
 *  \remark
 *  Verification is performed only it has been enabled (see #ddca_enable_verify()) and
 *  **verified_value** is set.
 *  \remark
 *  A verified value is returned if either the status code is either 0 (success),
 *  or **DDCRC_VERIFY**, i.e. the write succeeded but verification failed.
 */
DDCA_Status
ddca_set_table_vcp_value_verify(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code,
      DDCA_Table_Vcp_Value *   new_value,
      DDCA_Table_Vcp_Value **  verified_value_loc);


/** Sets a table VCP value.
 *  Optionally returns the value set by reading the feature code after writing.
 *
 *  \param[in]   ddca_dh             display handle
 *  \param[in]   feature_code        feature code
 *  \param[in]   new_value           value to set
 *  \return      status code
 */
DDCA_Status
ddca_set_table_vcp_value(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code,
      DDCA_Table_Vcp_Value *   new_value);


/** Sets a VCP value of any type.
 *  Optionally returns the values se by reading the feature code after writing.
 *
 *  \param[in]   ddca_dh        display handle
 *  \param[in]   feature_code   feature code
 *  \param[in]   new_value      value to set
 *  \param[out]  verified_value where to return verified value
 *  \return      status code
 *
 *  \remark
 *  Verification is performed only it has been enabled (see #ddca_enable_verify()) and
 *  **verified_value** is set.
 *  \remark
 *  A verified value is returned if either the status code is either 0 (success),
 *  or **DDCRC_VERIFY**, i.e. the write succeeded but verification failed.
 */
DDCA_Status
ddca_set_any_vcp_value_verify(
      DDCA_Display_Handle     ddca_dh,
      DDCA_Vcp_Feature_Code   feature_code,
      DDCA_Any_Vcp_Value *    new_value,
      DDCA_Any_Vcp_Value **   verified_value);


/** Sets a VCP value of any type.
 *
 *  \param[in]   ddca_dh        display handle
 *  \param[in]   feature_code   feature code
 *  \param[in]   new_value      value to set
 *  \return      status code
 */
DDCA_Status
ddca_set_any_vcp_value(
      DDCA_Display_Handle     ddca_dh,
      DDCA_Vcp_Feature_Code   feature_code,
      DDCA_Any_Vcp_Value *    new_value);


//
// Get or set multiple values
//
// These functions provide an API version of the **dumpvcp** and **loadvcp**
// commands.
//

/** Returns a string containing monitor identification and values
 *  for all detected features that should be saved when a monitor is
 *  calibrated and restored when the calibration is applied.
 *
 *  @param[in]  ddca_dh                   display handle
 *  @param[out] profile_values_string_loc address at which to return string
 *  @return     status code
 */
DDCA_Status
ddca_get_profile_related_values(
      DDCA_Display_Handle  ddca_dh,
      char**               profile_values_string_loc);

/** Sets multiple feature values for a specified monitor.
 *  The monitor identification and feature values are
 *  encoded in the string.
 *
 *  @param[in] profile_values_string address at which to return string
 *  @return     status code
 */
DDCA_Status
ddca_set_profile_related_values(
      char *               profile_values_string);


//
// Experimental - Not for public use
//

DDCA_Status
ddca_start_get_any_vcp_value(
      DDCA_Display_Handle         ddca_dh,
      DDCA_Vcp_Feature_Code       feature_code,
      DDCA_Vcp_Value_Type_Parm    call_type,
      DDCA_Notification_Func      callback_func);


// unimplemented
/** Registers a callback function to call when a VCP value changes */
DDCA_Status
ddca_register_callback(
      DDCA_Notification_Func func,
      uint8_t                callback_options);   // type is a placeholder

DDCA_Status
ddca_pass_callback(
      Simple_Callback_Func  func,
      int                   parm
      );

// unimplemeted
DDCA_Status
ddca_queue_get_non_table_vcp_value(
      DDCA_Display_Handle      ddca_dh,
      DDCA_Vcp_Feature_Code    feature_code
);


#ifdef __cplusplus
}
#endif
#endif /* DDCUTIL_C_API_H_ */
