#==========================================================================
#  TKENV.TCL -
#            part of the Tcl/Tk windowing environment of
#                            OMNeT++
#==========================================================================

#----------------------------------------------------------------#
#  Copyright (C) 1992,99 Andras Varga
#  Technical University of Budapest, Dept. of Telecommunications,
#  Stoczek u.2, H-1111 Budapest, Hungary.
#
#  This file is distributed WITHOUT ANY WARRANTY. See the file
#  `license' for details on this and other legal matters.
#----------------------------------------------------------------#

#
# Load library files
#
if [info exist OMNETPP_TKENV_DIR] {

   set dir $OMNETPP_TKENV_DIR

   source [file join $dir icons.tcl]
   source [file join $dir widgets.tcl]
   source [file join $dir balloon.tcl]
   source [file join $dir dialogs.tcl]
   source [file join $dir windows.tcl]
   source [file join $dir objinsp.tcl]
   source [file join $dir modinsp1.tcl]
   source [file join $dir modinsp2.tcl]
   source [file join $dir gateinsp.tcl]
   source [file join $dir statinsp.tcl]
   source [file join $dir menuproc.tcl]
   source [file join $dir main.tcl]

   # this is a temporary solution: hardcoded plugin directory
   set OMNETPP_PLUGIN_PATH "$OMNETPP_TKENV_DIR/plugins"

} else {
   set OMNETPP_PLUGIN_PATH "./plugins"
}

#
# Execute startup code
#
proc start_tkenv {} {
  global OMNETPP_BITMAP_PATH
  global OMNETPP_PLUGIN_PATH

  wm withdraw .
  checkVersion
  generic_bindings
  font_bindings
  init_balloons
  create_omnetpp_window
  puts ""
  load_bitmaps $OMNETPP_BITMAP_PATH
  load_plugins $OMNETPP_PLUGIN_PATH
  calibrate_animdelay
}

