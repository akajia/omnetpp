#==========================================================================
#  DIALOGS.TCL -
#            part of the Tcl/Tk windowing environment of
#                            OMNeT++
#==========================================================================

#----------------------------------------------------------------#
#  Copyright (C) 1992-2003 Andras Varga
#
#  This file is distributed WITHOUT ANY WARRANTY. See the file
#  `license' for details on this and other legal matters.
#----------------------------------------------------------------#

#------
# Parts of this file were created using Stewart Allen's Visual Tcl (vtcl)
#------

#===================================================================
#    HELPER/GUI PROCEDURES
#===================================================================

proc inputbox {title msg variable} {
    # This procedure displays a dialog box, waits for a button in the dialog
    # to be invoked, then returns the index of the selected button.

    upvar $variable var

    set w .inputbox
    createOkCancelDialog $w $title

    label $w.f.l -text $msg
    entry $w.f.e -highlightthickness 0
    pack $w.f.l -anchor w -expand 0 -fill none -padx 2 -pady 2 -side top
    pack $w.f.e -anchor w -expand 1 -fill x -padx 2 -pady 2 -side top
    $w.f.e insert 0 $var
    focus $w.f.e

    if [execOkCancelDialog $w] {
        set var [$w.f.e get]
        destroy $w
        return 1
    }
    destroy $w
    return 0
}

proc messagebox {title msg icon type} {
    return [tk_messageBox -title $title -message $msg -icon $icon -type $type]
}

proc comboSelectionDialog {title text label variable list} {
    set w .combodialog
    createOkCancelDialog $w $title

    upvar $variable var


    label $w.f.m -text $text -anchor w -justify left
    label-combo $w.f.c $label $list $var
    pack $w.f.m -fill x -padx 2 -pady 2 -side top
    pack $w.f.c -fill x -padx 2 -pady 2 -side top
    focus $w.f.c.e

    if [execOkCancelDialog $w] {
        set var [$w.f.c.e cget -value]
        destroy $w
        return 1
    }
    destroy $w
    return 0
}

# FIXME potentially obsolete:
# proc listboxSelectionDialog {title text list} {
#
#    set w .listdialog
#    createOkCancelDialog $w $title
#
#    label $w.f.label -text $text -justify left
#    pack $w.f.label -anchor w -expand 0 -fill none -padx 3m -pady 3m -side top
#
#    frame $w.f.main
#    scrollbar $w.f.main.sb -command "$w.f.main.list yview"
#    listbox $w.f.main.list  -height 6 -yscrollcommand "$w.f.main.sb set"
#    pack $w.f.main.sb -anchor center -expand 0 -fill y -side right
#    pack $w.f.main.list  -anchor center -expand 1 -fill both  -side left
#    pack $w.f.main  -anchor center -expand 1 -fill both -side top
#
#    set lb $w.f.main.list
#    foreach i $list {
#       $lb insert end $i
#    }
#    $lb selection set 0
#
#    # Configure dialog
#    bind $lb <Double-Button-1> "$w.buttons.okbutton invoke"
#    bind $lb <Key-Return> "$w.buttons.okbutton invoke"
#
#    focus $lb
#
#    if [execOkCancelDialog $w] {
#       if {[$lb curselection] != ""} {
#           set selection [$lb get [$lb curselection]]
#       } else {
#          set selection ""
#       }
#       destroy $w
#       return $selection
#    }
#    destroy $w
#    return ""
#}

proc inspectfromlistbox_insp {lb type} {
    set sel [$lb curselection]
    if {$sel != ""} {
        set ptr [lindex [$lb get $sel] 0]
        opp_inspect $ptr $type
    }
}

proc inspectfromlistbox {title text type fillistbox_args} {

    set w .listdialog
    createOkCancelDialog $w $title

    label $w.f.label -text $text -justify left
    pack $w.f.label -anchor w -expand 0 -fill none -padx 3m -pady 3m -side top

    frame $w.f.main
    scrollbar $w.f.main.sb -command "$w.f.main.list yview"
    listbox $w.f.main.list  -height 10 -yscrollcommand "$w.f.main.sb set" -width 60
    pack $w.f.main.sb -anchor center -expand 0 -fill y -side right
    pack $w.f.main.list  -anchor center -expand 1 -fill both  -side left
    pack $w.f.main  -anchor center -expand 1 -fill both -side top

    set lb $w.f.main.list
    eval opp_fill_listbox $lb $fillistbox_args
    $lb selection set 0

    button $w.buttons.inspect -text "Open inspector" -command "inspectfromlistbox_insp $lb \{$type\}; after 500 \{raise $w; focus $lb\}"
    pack $w.buttons.inspect -side top -anchor e -padx 2

    $w.buttons.okbutton config -text "Close"

    # Configure dialog
    bind $lb <Double-Button-1> "$w.buttons.inspect invoke"
    bind $lb <Key-Return> "$w.buttons.inspect invoke"

    focus $lb

    execOkCancelDialog $w
    destroy $w
}

proc display_stopdialog {mode} {
    # Create a dialog that can be used to stop a running simulation
    global opp fonts

    # 1. Create and configure dialog
    set w .stopdialog
    toplevel $w
    wm title $w {Running...}
    wm transient $w [winfo toplevel [winfo parent $w]]
    wm protocol $w WM_DELETE_WINDOW {opp_stopsimulation}
    # bind $w <Visibility> "raise $w"  ;# Keep modal window on top -- not good! (obscures error dialogs)

    button $w.stopbutton  -text {STOP!} -background red -activebackground red \
          -borderwidth 6 \
          -font $fonts(big)  \
          -command {opp_stopsimulation}
    if {$mode == "with_update"} {
       button $w.updatebutton  -text {Update object inspectors} \
          -borderwidth 3 \
          -command {opp_updateinspectors}
       pack $w.stopbutton -fill both -expand 1 -side top
       pack $w.updatebutton -fill x -side bottom
    } else {
       pack $w.stopbutton -fill both -expand 1
    }

    bind $w <Return> "opp_stopsimulation"
    bind $w <Escape> "opp_stopsimulation"
    bind $w <F8>     "opp_stopsimulation"

    # 2. Center window
    center $w

    # 3. Set a grab and claim the focus too.

    set opp(oldFocus) [focus]
    set opp(oldGrab) [grab current $w]
    grab $w
    focus $w.stopbutton
}

proc remove_stopdialog {} {
    # Remove the dialog created by display_stopdialog

    global opp
    set w .stopdialog

    # Restore the focus before deleting the window, since otherwise the
    # window manager may take the focus away so we can't redirect it.
    # Finally, restore any grab that was in effect.

    catch {focus $opp(oldFocus)}
    destroy $w
    if {$opp(oldGrab) != ""} {
        grab $opp(oldGrab)
    }
}

proc options_dialog {} {

    global opp

    set w .optionsdialog

    createOkCancelDialog $w {Simulation options}

    frame $w.f.f1 -relief groove -borderwidth 2
    label-entry $w.f.f1.stepdelay       {Delay for slow execution}
    label-entry $w.f.f1.updfreq_fast    {Update freq. for Fast Run (events)}
    label-entry $w.f.f1.updfreq_express {Update freq. for Express Run (events)}
    $w.f.f1.stepdelay.l config -width 0
    $w.f.f1.updfreq_fast.l config -width 0
    $w.f.f1.updfreq_express.l config -width 0
    $w.f.f1.stepdelay.e config -width 8
    $w.f.f1.updfreq_fast.e config -width 8
    $w.f.f1.updfreq_express.e config -width 8
    pack $w.f.f1.stepdelay -anchor w -expand 0 -fill x
    pack $w.f.f1.updfreq_fast -anchor w -expand 0 -fill x
    pack $w.f.f1.updfreq_express -anchor w -expand 0 -fill x

    frame $w.f.f2 -relief groove -borderwidth 2
    checkbutton $w.f.f2.usemainwin -text {Use main window for module output} -variable opp(usemainwin)
    checkbutton $w.f.f2.banners -text {Print event banners} -variable opp(banners)
    checkbutton $w.f.f2.bkpts -text {Stop on breakpoint() calls} -variable opp(bkpts)
    pack $w.f.f2.usemainwin -anchor w
    pack $w.f.f2.banners -anchor w
    pack $w.f.f2.bkpts -anchor w

    frame $w.f.f3 -relief groove -borderwidth 2
    checkbutton $w.f.f3.anim -text {Animate messages} -variable opp(anim)
    checkbutton $w.f.f3.nextev -text {Show next event markers} -variable opp(nextev)
    checkbutton $w.f.f3.sdarrows -text {Show arrows for sendDirect()} -variable opp(sdarrows)
    checkbutton $w.f.f3.animmeth -text {Animate method calls} -variable opp(animmeth)
    checkbutton $w.f.f3.msgnam -text {Message names during animation} -variable opp(msgnam)
    checkbutton $w.f.f3.msgcol -text {Coloring by message kind} -variable opp(msgcol)
    commentlabel $w.f.f3.c {Color code (message->kind() mod 7):
     0=red 1=green 2=blue 3=white
     4=yellow 5=cyan 6=magenta 7=black}
    label-scale $w.f.f3.speed {Animation speed:}
    $w.f.f3.speed.e config -length 200 -from 0 -to 3 \
                           -resolution 0.01 -variable opp(speed)
    pack $w.f.f3.anim -anchor w
    pack $w.f.f3.nextev -anchor w
    pack $w.f.f3.sdarrows -anchor w
    pack $w.f.f3.animmeth -anchor w
    pack $w.f.f3.msgnam -anchor w
    pack $w.f.f3.msgcol -anchor w
    pack $w.f.f3.c -anchor w
    pack $w.f.f3.speed -anchor c

    pack $w.f.f2 -anchor center -expand 1 -fill both -ipadx 0 -ipady 0 -padx 10 -pady 10 -side top
    pack $w.f.f3 -anchor center -expand 1 -fill both -ipadx 0 -ipady 0 -padx 10 -pady 10 -side top
    pack $w.f.f1 -anchor center -expand 1 -fill both -ipadx 0 -ipady 0 -padx 10 -pady 10 -side top

    # Configure dialog
    $w.f.f1.updfreq_fast.e insert 0 [opp_getsimoption updatefreq_fast]
    $w.f.f1.updfreq_express.e insert 0 [opp_getsimoption updatefreq_express]
    $w.f.f1.stepdelay.e insert 0 [opp_getsimoption stepdelay]
    set opp(usemainwin) [opp_getsimoption use_mainwindow]
    set opp(banners)    [opp_getsimoption print_banners]
    set opp(anim)       [opp_getsimoption animation_enabled]
    set opp(nextev)     [opp_getsimoption nexteventmarkers]
    set opp(sdarrows)   [opp_getsimoption senddirect_arrows]
    set opp(animmeth)   [opp_getsimoption anim_methodcalls]
    set opp(msgnam)     [opp_getsimoption animation_msgnames]
    set opp(msgcol)     [opp_getsimoption animation_msgcolors]
    set opp(speed)      [opp_getsimoption animation_speed]
    set opp(bkpts)      [opp_getsimoption bkpts_enabled]

    focus $w.f.f2.usemainwin

    if [execOkCancelDialog $w] {
        opp_setsimoption stepdelay [$w.f.f1.stepdelay.e get]
        opp_setsimoption updatefreq_fast [$w.f.f1.updfreq_fast.e get]
        opp_setsimoption updatefreq_express [$w.f.f1.updfreq_express.e get]
        opp_setsimoption use_mainwindow      $opp(usemainwin)
        opp_setsimoption print_banners       $opp(banners)
        opp_setsimoption animation_enabled   $opp(anim)
        opp_setsimoption nexteventmarkers    $opp(nextev)
        opp_setsimoption senddirect_arrows   $opp(sdarrows)
        opp_setsimoption anim_methodcalls    $opp(animmeth)
        opp_setsimoption animation_msgnames  $opp(msgnam)
        opp_setsimoption animation_msgcolors $opp(msgcol)
        opp_setsimoption animation_speed     $opp(speed)
        opp_setsimoption bkpts_enabled       $opp(bkpts)
    }
    destroy $w
}

proc rununtil_dialog {time_var event_var mode_var} {

    global opp
    upvar $time_var time_var0
    upvar $event_var event_var0
    upvar $mode_var mode_var0

    set w .rununtil

    createOkCancelDialog $w {Run until}

    label-entry $w.f.time  {Simulation time to stop at:}
    label-entry $w.f.event {Event number to stop at:}
    label-combo $w.f.mode  {Running mode:} \
                 {{Normal} {Fast (rare updates)} {Express (tracing off)}}

    foreach i {time event mode} {
       $w.f.$i.l configure -width 24
       $w.f.$i.e configure -width 18
       pack $w.f.$i -anchor w
    }

    pack $w.f -anchor center -expand 1 -fill both -padx 10 -pady 10 -side top

    focus $w.f.time.e

    if [execOkCancelDialog $w] {
        set time_var0  [$w.f.time.e get]
        set event_var0 [$w.f.event.e get]
        set mode_var0  [lindex [$w.f.mode.e cget -value] 0]
        destroy $w
        return 1
    }
    destroy $w
    return 0
}



# findDialog --
#
proc findDialog {w} {

    global tmp config

    set tmp(case-sensitive)  $config(editor-case-sensitive)
    set tmp(whole-words)     $config(editor-whole-words)
    set tmp(regexp)          $config(editor-regexp)
    set tmp(backwards)       $config(editor-backwards)

    # dialog should be child of the window which contains the text widget
    set dlg [winfo toplevel $w].dlg
    if {$dlg=="..dlg"} {set dlg .dlg}

    # create dialog with OK and Cancel buttons
    set title "Find"
    createOkCancelDialog $dlg $title

    # add entry fields
    label-entry $dlg.f.find "Find string:"
    pack $dlg.f.find  -expand 0 -fill x -side top

    checkbutton $dlg.f.regexp -text {regular expression} -variable tmp(regexp)
    pack $dlg.f.regexp  -anchor w -side top

    checkbutton $dlg.f.case -text {case sensitive} -variable tmp(case-sensitive)
    pack $dlg.f.case  -anchor w -side top

    checkbutton $dlg.f.words -text {whole words only} -variable tmp(whole-words)
    pack $dlg.f.words  -anchor w -side top

    checkbutton $dlg.f.backwards -text {search backwards} -variable tmp(backwards)
    pack $dlg.f.backwards  -anchor w -side top

    $dlg.f.find.e insert 0 $config(editor-findstring)
    $dlg.f.find.e select range 0 end

    focus $dlg.f.find.e

    # exec the dialog, extract its contents if OK was pressed, then delete dialog
    if {[execOkCancelDialog $dlg] == 1} {
        set findstring [$dlg.f.find.e get]

        set case $tmp(case-sensitive)
        set words $tmp(whole-words)
        set regexp $tmp(regexp)
        set backwards $tmp(backwards)

        set config(editor-findstring) $findstring
        set config(editor-case-sensitive) $case
        set config(editor-whole-words) $words
        set config(editor-regexp) $regexp
        set config(editor-backwards) $backwards

        destroy $dlg
        doFind $w $findstring $case $words $regexp $backwards
   }
   catch {destroy $dlg}
}


# findNext --
#
# find next occurrence of the string in the editor window
#
proc findNext {w} {
    global config

    set findstring   $config(editor-findstring)
    set case         $config(editor-case-sensitive)
    set words        $config(editor-whole-words)
    set regexp       $config(editor-regexp)
    set backwards    $config(editor-backwards)

    doFind $w $findstring $case $words $regexp $backwards
}


# doFind --
#
#
proc doFind {w findstring case words regexp backwards} {
    if {[_doFind $w $findstring $case $words $regexp $backwards] == ""} {
        tk_messageBox -parent [winfo toplevel $w] -title "Find" -icon warning \
                      -type ok -message "'$findstring' not found."
    }
}

# _doFind --
#
# Internal proc for doFind and doReplace.
#
# Finds the given string, positions the cursor after its last char,
# and returns the length. Returns empty string if not found.
#
proc _doFind {w findstring case words regexp backwards} {

    # remove previous highlights
    $w tag remove SELECT 0.0 end

    # find the string
    set cur "insert"
    set initialcur $cur
    while 1 {
        # do search
        if {$backwards} {
            if {$case && $regexp} {
                set cur [$w search -count length -backwards -regexp -- $findstring $cur 1.0]
            } elseif {$case} {
                set cur [$w search -count length -backwards -- $findstring $cur 1.0]
            } elseif {$regexp} {
                set cur [$w search -count length -backwards -nocase -regexp -- $findstring $cur 1.0]
            } else {
                set cur [$w search -count length -backwards -nocase -- $findstring $cur 1.0]
            }
        } else {
            if {$case && $regexp} {
                set cur [$w search -count length -regexp -- $findstring $cur end]
            } elseif {$case} {
                set cur [$w search -count length -- $findstring $cur end]
            } elseif {$regexp} {
                set cur [$w search -count length -nocase -regexp -- $findstring $cur end]
            } else {
                set cur [$w search -count length -nocase -- $findstring $cur end]
            }
        }

        # exit if not found
        if {$cur == ""} {
            break
        }

        # allow exit loop only if we moved from initial cursor position
        if {![$w compare "$cur  + $length chars" == $initialcur]} {
            # if 'whole words' and we are not at beginning of a word, continue searching
            if {!$words} {
                break
            }
            if {[$w compare $cur == "$cur wordstart"] && \
                [$w compare "$cur + $length char" == "$cur wordend"]} {
                break
            }
        }

        # move cur so that we find next/prev occurrence
        if {$backwards} {
            set cur "$cur - 1 char"
        } else {
            set cur "$cur + 1 char"
        }
    }

    # check if found
    if {$cur == ""} {
        return ""
    }

    # highlight it and return length
    $w tag add SELECT $cur "$cur + $length chars"
    $w mark set insert "$cur + $length chars"
    $w see insert

    return $length
}


# filteredobjectlist_dialog --
#
# Implements the "Find/inspect objects" dialog
#
proc filteredobjectlist_dialog {} {
    set w .objdlg
    createCloseDialog $w "Find/inspect objects"

    # two panels: $w.f.filter is the upper panel for filters, and
    # $w.f.main is the lower one with the listbox.

    # panel for filters
    frame $w.f.filter
    pack $w.f.filter -anchor center -expand 0 -fill x -side top

    label $w.f.filter.title -text "Filter list of all objects in the simulation:" -justify left -anchor w
    pack $w.f.filter.title -anchor w -expand 1 -fill x -side top

    frame $w.f.filter.pars -relief groove -bd 2
    pack $w.f.filter.pars -anchor center -expand 1 -fill x -side top
    set wfiltpars $w.f.filter.pars
    frame $wfiltpars.class
    frame $wfiltpars.name
    frame $wfiltpars.order
    pack $wfiltpars.class -anchor center -expand 0 -fill both -side left
    pack $wfiltpars.name $wfiltpars.order -anchor center -expand 1 -fill both -side left
    pack $wfiltpars.order -anchor center -expand 0 -fill both -side left

    label $wfiltpars.class.label -text "Class:" -justify left
    pack $wfiltpars.class.label -anchor w -expand 0 -fill none -side top
    label $wfiltpars.name.label -text "Object name (full path):" -justify left
    pack $wfiltpars.name.label -anchor w -expand 0 -fill none -side top
    label $wfiltpars.order.label -text "Ordered by:" -justify left
    pack $wfiltpars.order.label -anchor w -expand 0 -fill none -side top

    combo $wfiltpars.class.entry [concat {{}} [getClassNames]]
    pack $wfiltpars.class.entry -anchor w -expand 0 -fill x -side top
    entry $wfiltpars.name.entry
    pack $wfiltpars.name.entry -anchor w -expand 0 -fill both -side top
    combo $wfiltpars.order.entry {{Class} {Full name} {Name}}
    pack $wfiltpars.order.entry -anchor w -expand 0 -fill x -side top

    set helptext "Class and object name accepts wildcards:\n\
                  *=any string, ?=any char, {a-z}=any char from set, {^a-z}=any char NOT from the set.\n\
                  Example: *.node\[{5-8}].*.histogram\n\
                  To match parts of the string, use * before and after the string. Example: *Packet*\n\
                  Match is case sensitive -- 'a' and 'A' count as different."

    label $w.f.filter.help -text $helptext -justify left -anchor w
    pack $w.f.filter.help -anchor w -expand 1 -fill x -side top

    frame $w.f.filter.buttons
    pack $w.f.filter.buttons -anchor center -expand 1 -fill x -side top
    button $w.f.filter.buttons.refresh -text Refresh -command "filteredobjectlist_refresh $w"
    pack $w.f.filter.buttons.refresh -anchor e -expand 0 -fill none -side top


    # number of objects
    label $w.f.numobj -text "Found 0 objects" -justify left -anchor w
    pack $w.f.numobj -anchor w -expand 0 -fill x -side top

    # panel for listbox
    frame $w.f.main
    scrollbar $w.f.main.sb -command "$w.f.main.list yview"
    listbox $w.f.main.list  -height 10 -yscrollcommand "$w.f.main.sb set" -width 60
    pack $w.f.main.sb -anchor center -expand 0 -fill y -side right
    pack $w.f.main.list  -anchor center -expand 1 -fill both  -side left
    pack $w.f.main  -anchor center -expand 1 -fill both -side top

    set lb $w.f.main.list

    set type "(default)"
    #button $w.buttons.inspect -text "Open inspector" -command "inspectfromlistbox_insp $lb \{$type\}; after 500 \{raise $w; focus $lb\}"
    #pack $w.buttons.inspect -side top -anchor e -padx 2

    # leave listbox empty -- filling it with all objects might take too long

    # Configure dialog
    bind $wfiltpars.class.entry.entry <Return> "$w.f.filter.buttons.refresh invoke"
    bind $wfiltpars.name.entry <Return> "$w.f.filter.buttons.refresh invoke"
    bind $wfiltpars.order.entry.entry <Return> "$w.f.filter.buttons.refresh invoke"
    bind $lb <Double-Button-1> "inspectfromlistbox_insp $lb \{$type\}; after 500 \{raise $w; focus $lb\}"
    bind $lb <Key-Return> "inspectfromlistbox_insp $lb \{$type\}; after 500 \{raise $w; focus $lb\}"
    bind $lb <Button-3> "filteredobjectlist_popup $w \[lindex \[$lb get @%x,%y\] 0\] %X %Y"

    focus $wfiltpars.name.entry

    ## potentially useful stuff:
    #set type [listboxSelectionDialog {Choose Type...} {Select inspector type.} [opp_inspectortype all]]
    #if {$type == ""} return

    execCloseDialog $w
    destroy $w
}

# getClassNames --
#
# helper proc for filteredobjectlist_dialog
#
proc getClassNames {} {
    # FIXME modules and channels are not registered as classes!
    set classes [concat [opp_getchildobjects [opp_object_classes]] \
                        [opp_getchildobjects [opp_object_channeltypes]] \
                        [opp_getchildobjects [opp_object_moduletypes]] ]
    # get the names
    set classnames {}
    foreach classptr $classes {
        lappend classnames [opp_getobjectfullname $classptr]
    }
    lappend classnames {cWatch}  ;# FIXME some types are not registered
    lappend classnames {cOutVector}  ;# FIXME some types are not registered

    return [lsort -dictionary $classnames]
}

# filteredobjectlist_refresh --
#
# helper proc for filteredobjectlist_dialog
#
proc filteredobjectlist_refresh {w} {
    set class [$w.f.filter.pars.class.entry get]
    set name [$w.f.filter.pars.name.entry get]
    set orderby [$w.f.filter.pars.order.entry get]

    # tk_messageBox -type ok -title INFO -icon info -message "$class:$name:$orderby"

    # get list
    set objlist [opp_getsubobjectsfilt [opp_object_systemmodule] $class $name $orderby]
    set num [llength $objlist];

    # ask user if too many...
    set viewall "yes"
    if {$num > 100000} {
        set viewall [tk_messageBox -message "Your query matched $num objects. \
                     Do you want to display all of them (it might take a while)? \
                     Clicking \"No\" will display the first 100,000 only." \
                       -title "Too many hits" -icon question -type yesno -parent $w]
    }
    
    # clear listbox
    set lb $w.f.main.list
    $lb delete 0 end

    # insert into listbox
    # FIXME meanwhile, doctor the info string -- cut off object name and class if exists...
    if {$viewall == "yes"} {
        $w.f.numobj config -text "Found $num objects"
        foreach ptr $objlist {
            # FIXME doctor the info string -- cut off object name and class if exists...
            set classname [opp_getobjectclassname $ptr]
            set fullpath [opp_getobjectfullpath $ptr]
            set infostr0 [opp_getobjectinfostring $ptr]
            regsub "^.*\\($classname\\)" $infostr0 "" infostr

            $lb insert end "$ptr ($classname)  $fullpath    $infostr"
        }
    } else {    
        set i 0
        $w.f.numobj config -text "Found $num objects, first 100,000 displayed"
        foreach ptr $objlist {
            set classname [opp_getobjectclassname $ptr]
            set fullpath [opp_getobjectfullpath $ptr]
            set infostr0 [opp_getobjectinfostring $ptr]
            regsub "^.*\\($classname\\)" $infostr0 "" infostr

            $lb insert end "$ptr ($classname)  $fullpath    $infostr"

            incr i
            if {$i > 100000} {break}
        }
    }
    $lb selection set 0
}

# filteredobjectlist_popup --
#
# helper procedure for filteredobjectlist_dialog -- creates popup menu
#
proc filteredobjectlist_popup {w ptr X Y} {
    if {$ptr==""} return
    set insptypes [opp_supported_insp_types $ptr]

    set lb $w.f.main.list

    set p $w.popup
    catch {destroy $p}
    menu $p -tearoff 0
    foreach type $insptypes {
       $p add command -label "$type..." -command "opp_inspect $ptr \{$type\}; after 500 \{raise $w; focus $lb\}"
    }
    $p post $X $Y
}

