#==========================================================================
#  TREEMGR.TCL -
#            part of the GNED, the Tcl/Tk graphical topology editor of
#                            OMNeT++
#
#   By Andras Varga
#
#==========================================================================

#----------------------------------------------------------------#
#  Copyright (C) 1992,99 Andras Varga
#  Technical University of Budapest, Dept. of Telecommunications,
#  Stoczek u.2, H-1111 Budapest, Hungary.
#
#  This file is distributed WITHOUT ANY WARRANTY. See the file
#  `license' for details on this and other legal matters.
#----------------------------------------------------------------#

#-------------- temp solution: -----------------
# FIXME: rather include new icons into icons.tcl
set files [glob -nocomplain -- {icons/*_vs.gif}]
foreach f $files {
  set name [string tolower [file tail [file rootname $f]]]
  if [catch {image type $name}] {
     puts -nonewline "$name "
     image create photo $name -file $f
  }
}
puts ""

proc dispsel {} {
    global ned
#FIXME: debug proc
    foreach i [array names ned "*,selected"] {
       puts "dbg: ned($i)=$ned($i)"
    }
}
#-----------------------------------------------

# initTreeManager --
#
#
proc initTreeManager {} {
    global gned

    Tree:init $gned(manager).tree

    #
    # bindings for the tree
    #
    bind $gned(manager).tree <Button-1> {
        catch {destroy .popup}
        set key [Tree:nodeat %W %x %y]
        if {$key!=""} {
            Tree:setselection %W $key
        }
    }

    bind $gned(manager).tree <Double-1> {
        set key [Tree:nodeat %W %x %y]
        if {$key!=""} {
            # Tree:toggle %W $key
            treemanagerDoubleClick $key
        }
    }

    bind $gned(manager).tree <Button-3> {
        set key [Tree:nodeat %W %x %y]
        if {$key!=""} {
            Tree:setselection %W $key
            treemanagerPopup $key %X %Y
        }
    }

    #
    # bindings for the resize bar
    #
    bind $gned(manager).resize <Button-1> {
        global mouse
        set mouse(x) %x
    }

    bind $gned(manager).resize <ButtonRelease-1> {
        global mouse
        set dx [expr %x-$mouse(x)]

        set width [$gned(manager).tree cget -width]
        set width [expr $width+$dx]
        $gned(manager).tree config -width $width
    }
}

# updateTreeManager --
#
# Redraws the manager window (left side of main window).
#
proc updateTreeManager {} {
    global gned

    Tree:build $gned(manager).tree
}

proc treemanagerDoubleClick {key} {
    global ned

    set type $ned($key,type)
    if {$type=="module"} {
        openModuleOnCanvas $key
    } else {
        tk_messageBox -icon warning -type ok \
            -message "Opening a '$type' on canvas is not implemented yet."
    }
}

proc treemanagerPopup {key x y} {
    global ned

    catch {destroy .popup}
    menu .popup -tearoff 0
    switch $ned($key,type) {
        nedfile {nedfilePopup $key}
        module  {modulePopup $key}
        default {defaultPopup $key}
    }
    .popup post $x $y
}

proc nedfilePopup {key} {
    global ned

    foreach i {
      {cascade -menu .popup.newmenu -label {New} -underline 0}
      {separator}
      {command -command "saveNED $key" -label {Save} -underline 0}
      {command -command "puts {Not implemented}" -label {Save As...} -underline 1}
      {command -command "puts {Not implemented}" -label {Close} -underline 0}
      {separator}
      {command -command "displayCodeForItem $key" -label {Show NED code...} -underline 0}
      {separator}
      {command -command "deleteItem $key; updateTreeManager" -label {Delete} -underline 0}
    } {
       eval .popup add $i
    }

    menu .popup.newmenu -tearoff 0
    foreach i {
      {command -command "addItem imports $key; updateTreeManager" -label {imports} -underline 0}
      {command -command "addItem channel $key; updateTreeManager" -label {channel} -underline 0}
      {command -command "addItem simple $key;  updateTreeManager" -label {simple}  -underline 0}
      {command -command "addItem module $key;  updateTreeManager" -label {module}  -underline 0}
      {command -command "addItem network $key; updateTreeManager" -label {network} -underline 0}
    } {
       eval .popup.newmenu add $i
    }
}

proc modulePopup {key} {
    global ned
    # FIXME:
    foreach i {
      {command -command "openModuleOnCanvas $key" -label {Open on canvas} -underline 0}
      {command -command "displayCodeForItem $key" -label {Show NED code...} -underline 0}
      {separator}
      {command -command "deleteItem $key; updateTreeManager" -label {Delete} -underline 0}
    } {
       eval .popup add $i
    }
}

proc defaultPopup {key} {
    global ned
    # FIXME:
    foreach i {
      {command -command "displayCodeForItem $key" -label {Show NED fragment...} -underline 0}
      {separator}
      {command -command "deleteItem $key; updateTreeManager" -label {Delete} -underline 0}
    } {
       eval .popup add $i
    }
}

#--------------------------------------
proc displayCodeForItem {key} {
    global ned fonts

    if [info exist ned($key,name)] {
        set txt "$ned($key,type) $ned($key,name)"
    } else {
        set txt "$ned($key,type)"
    }


    # open file viewer/editor window
    set w .nedcode
    catch {destroy $w}

    # create widgets
    toplevel $w -class Toplevel
    wm focusmodel $w passive
    wm geometry $w 512x275
    wm maxsize $w 1009 738
    wm minsize $w 1 1
    wm overrideredirect $w 0
    wm resizable $w 1 1
    wm title $w "NED code -- $txt"

    frame $w.main
    scrollbar $w.main.sb -borderwidth 1 -command "$w.main.text yview"
    pack $w.main.sb -anchor center -expand 0 -fill y -side right
    text $w.main.text -width 60 -yscrollcommand "$w.main.sb set" -wrap none -font $fonts(fixed) -bg #c0c0c0
    pack $w.main.text -anchor center -expand 1 -fill both -side left

    frame $w.butt
    button $w.butt.close -text Close -command "destroy $w"
    pack $w.butt.close -anchor n -expand 0 -side right

    pack $w.butt -expand 0 -fill x -side bottom
    pack $w.main -expand 1 -fill both -side top

    # produce ned code and put it into text widget
    set nedcode [generateNed $key]
    $w.main.text insert end $nedcode
}


# getNodeInfo --
#
# This user-supplied function gets called by the tree widget to get info about
# tree nodes. The widget itself only stores the state (open/closed) of the
# nodes, everything else comes from this function.
#
proc getNodeInfo {w op {key {}}} {
    global ned ddesc

    switch $op {

      root {
        return 0
      }

      text {
        #DBG:
        set k "$key:"
        #set k ""

        if [info exist ned($key,name)] {
          return "$k$ned($key,type) $ned($key,name)"
        } else {
          return "$k$ned($key,type)"
        }
      }

      options {
        if [info exist ned($key,dirty)] {
          if {$ned($key,dirty)} {
             return "-fill #ff0000"
          } else {
             return ""
          }
        }
      }

      icon {
        set type $ned($key,type)
        if [info exist ddesc($type,treeicon)] {
          return $ddesc($type,treeicon)
        } else {
          return $ddesc(root,treeicon)
        }
      }

      parent {
        return $ned($key,parentkey)
      }

      children {
        # FIXME: ordering!
        return $ned($key,childrenkeys)
      }

      haschildren {
        return [expr [llength $ned($key,childrenkeys)]!=0]

        ## OLD CODE: only allow top-level components (modules, channels etc.) to be displayed
        # set type $ned($key,type)
        # if {$type=="root" || $type=="nedfile"} {
        #   return [expr [llength $ned($key,childrenkeys)]!=0]
        # } else {
        #   return 0
        #}
      }
    }
}


