/*--------------------------------------------------------------*
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.common.eventlog;

import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.core.runtime.Assert;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.omnetpp.common.engine.BigDecimal;
import org.omnetpp.common.virtualtable.IVirtualTableSelection;
import org.omnetpp.eventlog.engine.IEventLog;

/**
 * Selection that is published by event log editors and viewers.
 *
 * @author andras
 */
/*
 * TODO: change this class so that it can represent selection of
 * - event
 * - eventlogentry
 * - simulation time
 * - animation time
 * - timeline coordinate
 * - module
 * - submodule
 * - gate
 * - connection
 * - message
 * - animation primitive
 *
 * It should support multiple selected elements. For some of those it should support ranges where it is applicable.
 */
public class EventLogSelection implements IEventLogSelection, IVirtualTableSelection<Long>, IStructuredSelection, Cloneable {
	/**
	 * The input where this selection is.
	 */
	protected EventLogInput eventLogInput;

	/**
	 * The selected elements.
	 */
	protected ArrayList<Object> elements;

    public EventLogSelection(EventLogInput eventLogInput, ArrayList<Object> elements) {
        this.eventLogInput = eventLogInput;
        this.elements = elements;
    }

    public EventLogSelection(EventLogInput eventLogInput, ArrayList<Long> eventNumbers, ArrayList<BigDecimal> simulationTimes) {
		Assert.isTrue(eventLogInput != null);
		this.eventLogInput = eventLogInput;
		this.elements = new ArrayList<Object>();
		if (eventNumbers != null)
		    elements.addAll(eventNumbers);
		if (simulationTimes != null)
		    elements.addAll(simulationTimes);
	}

    public ArrayList<Long> getElements() {
		return getEventNumbers();
	}

	public Object getInput() {
		return eventLogInput;
	}

	public IEventLog getEventLog() {
		return eventLogInput.getEventLog();
	}

	public EventLogInput getEventLogInput() {
		return eventLogInput;
	}

	public ArrayList<Long> getEventNumbers() {
	    ArrayList<Long> eventNumbers = new ArrayList<Long>();
	    for (Object element : elements)
	        if (element instanceof Long)
	            eventNumbers.add((Long)element);
		return eventNumbers;
	}

	public Long getFirstEventNumber() {
        ArrayList<Long> eventNumbers = getEventNumbers();
		return eventNumbers.isEmpty() ? null : eventNumbers.get(0);
	}

    public ArrayList<BigDecimal> getSimulationTimes() {
        ArrayList<BigDecimal> simulationTimes = new ArrayList<BigDecimal>();
        for (Object element : elements)
            if (element instanceof BigDecimal)
                simulationTimes.add((BigDecimal)element);
        return simulationTimes;
    }

    public BigDecimal getFirstSimulationTime() {
        ArrayList<BigDecimal> simulationTimes = getSimulationTimes();
        return simulationTimes.isEmpty() ? null : simulationTimes.get(0);
    }

    public boolean isEmpty() {
		return elements.isEmpty();
	}

	@Override
	public EventLogSelection clone() {
		return new EventLogSelection(this.eventLogInput, elements);
	}

	@Override
	public boolean equals(Object o) {
		if (o == null || !(o instanceof EventLogSelection))
			return false;
		EventLogSelection other = (EventLogSelection)o;
		if (other.eventLogInput != eventLogInput)
			return false;
		if (other.elements.size() != elements.size())
			return false;
		for (int i = 0; i < elements.size(); i++)
			if (other.elements.get(i) != elements.get(i))
				return false;
		return true;
	}

	public Object getFirstElement() {
		if (elements.size() == 0)
			return null;
		else
			return elements.get(0);
	}

	public Iterator<Object> iterator() {
		return elements.iterator();
	}

	public int size() {
		return elements.size();
	}

	public Object[] toArray() {
		return elements.toArray();
	}

	public ArrayList<Object> toList() {
		return elements;
	}
}
