/*--------------------------------------------------------------*
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.animation.primitives;

import org.eclipse.draw2d.MouseEvent;
import org.eclipse.draw2d.MouseListener;
import org.omnetpp.animation.figures.AnimationCompoundModuleFigure;
import org.omnetpp.animation.widgets.AnimationCanvas;
import org.omnetpp.animation.widgets.AnimationController;
import org.omnetpp.common.eventlog.EventLogModule;
import org.omnetpp.figures.CompoundModuleFigure;
import org.omnetpp.figures.CompoundModuleLineBorder;
import org.omnetpp.figures.SubmoduleFigure;
import org.omnetpp.ned.model.DisplayString;

public class CreateModuleAnimation extends AbstractInfiniteAnimation {
	protected EventLogModule module;

	protected int parentModuleId;

	public CreateModuleAnimation(AnimationController animationController, EventLogModule module, int parentModuleId) {
		super(animationController);
		this.module = module;
		this.parentModuleId = parentModuleId;
	}

	@Override
	public void activate() {
	    super.activate();
	    // FIXME: we don't yet know if this module is really a compound module, so we might just superfluously create one
        final AnimationCanvas animationCanvas = animationController.getAnimationCanvas();
        final CompoundModuleFigure compoundModuleFigure = new CompoundModuleFigure();
        compoundModuleFigure.setBorder(new CompoundModuleLineBorder());
        compoundModuleFigure.setDisplayString(new DisplayString(""));
        compoundModuleFigure.setOpaque(true);
        compoundModuleFigure.addMouseListener(new MouseListener() {
            public void mouseDoubleClicked(MouseEvent me) {
            }

            public void mousePressed(MouseEvent me) {
            }

            public void mouseReleased(MouseEvent me) {
                animationCanvas.setSelectedElement(compoundModuleFigure, module);
            }
        });
        animationController.setFigure(module, compoundModuleFigure);
        if (animationCanvas.showsCompoundModule(module.getId())) {
            AnimationCompoundModuleFigure animationCompoundModuleFigure = animationCanvas.findAnimationCompoundModuleFigure(module.getId());
            if (animationCompoundModuleFigure != null)
                animationCompoundModuleFigure.setCompoundModuleFigure(compoundModuleFigure);
            else
                animationCanvas.addFigure(new AnimationCompoundModuleFigure(animationController, compoundModuleFigure, module.getId(), module.getFullPath()));
        }
        EventLogModule parentModule = getSimulation().getModuleById(parentModuleId);
		CompoundModuleFigure parentCompoundModuleFigure = getCompoundModuleFigure(parentModule);
        if (parentCompoundModuleFigure != null) {
			final SubmoduleFigure submoduleFigure = new SubmoduleFigure();
            submoduleFigure.setName(module.getFullName());
            submoduleFigure.setDisplayString(1.0f, new DisplayString(""));
            submoduleFigure.setRangeFigureLayer(parentCompoundModuleFigure.getBackgroundDecorationLayer());
			submoduleFigure.addMouseListener(new MouseListener() {
				public void mouseDoubleClicked(MouseEvent me) {
				    if (module.getSubmodules() != null) {
				        CompoundModuleFigure compoundModuleFigure = (CompoundModuleFigure)animationController.getFigure(module, CompoundModuleFigure.class);
				        AnimationCanvas animationCanvas = animationController.getAnimationCanvas();
                        if (!animationCanvas.showsCompoundModule(module.getId())) {
                            animationController.stopAnimation();
				            long eventNumber = animationController.getEventNumber();
                            animationCanvas.addShownCompoundModule(module.getId());
				            animationController.reloadAnimationPrimitives();
                            animationController.gotoEventNumber(eventNumber);
				        }
				        else
				            animationCanvas.reveal(compoundModuleFigure);
                    }
				}

				public void mousePressed(MouseEvent me) {
				}

				public void mouseReleased(MouseEvent me) {
				    animationCanvas.setSelectedElement(submoduleFigure, module);
				}
			});
			animationController.setFigure(module, submoduleFigure);
            parentCompoundModuleFigure.getSubmoduleLayer().add(submoduleFigure);
		}
	}

	@Override
	public void deactivate() {
        super.deactivate();
		EventLogModule parentModule = getSimulation().getModuleById(parentModuleId);
        CompoundModuleFigure parentCompoundModuleFigure = getCompoundModuleFigure(parentModule);
        if (parentCompoundModuleFigure != null) {
		    SubmoduleFigure submoduleFigure = (SubmoduleFigure)animationController.getFigure(module, SubmoduleFigure.class);
            animationController.setFigure(module, SubmoduleFigure.class, null);
		    parentCompoundModuleFigure.getSubmoduleLayer().remove(submoduleFigure);
		}
	}
}
