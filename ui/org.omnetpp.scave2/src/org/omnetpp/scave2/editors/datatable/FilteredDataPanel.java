package org.omnetpp.scave2.editors.datatable;

import static org.omnetpp.scave2.model.RunAttribute.EXPERIMENT;
import static org.omnetpp.scave2.model.RunAttribute.MEASUREMENT;
import static org.omnetpp.scave2.model.RunAttribute.REPLICATION;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.forms.widgets.FormToolkit;
import org.omnetpp.scave.engine.FileRunList;
import org.omnetpp.scave.engine.IDList;
import org.omnetpp.scave.engine.ResultFileList;
import org.omnetpp.scave.engine.RunList;
import org.omnetpp.scave.engine.StringMap;
import org.omnetpp.scave.engineext.ResultFileManagerEx;
import org.omnetpp.scave2.model.FilterHints;
import org.omnetpp.scave2.model.FilterParams;
import org.omnetpp.scave2.model.ScaveModelUtil;

/**
 * Displays a data table of vectors/scalars/histograms with filter
 * comboboxes.
 * 
 * This class is reusable, which means it only knows that it has to 
 * display an IDList belonging to a particular ResultFileManager,
 * and has absolutely no reference to the editor, or EMF model objects,
 * or any widgets outside -- nothing.
 * 
 * The user is responsible to keep contents up-to-date in case
 * ResultFileManager or IDList contents change. Refreshing can be
 * done by calling setIDList(). 
 *  
 * @author andras
 */
//XXX tables and filter should include the experiment, measurement, replication fields as well
//XXX filter should include expressions ("where load>10")
//XXX make filter panel foldable?
public class FilteredDataPanel extends Composite {
	private FilteringPanel filterPanel;
	private DataTable table;
	private IDList idlist; // the unfiltered data list
	private FormToolkit formToolkit = null;   //  @jve:decl-index=0:visual-constraint=""
	
	public FilteredDataPanel(Composite parent, int style, int type) {
		super(parent, style);
		initialize(type);
		configureFilterPanel();
	}

	public FilteringPanel getFilterPanel() {
		return filterPanel;
	}

	public DataTable getTable() {
		return table;
	}
	
	public void setIDList(IDList idlist) {
		this.idlist = idlist;
		updateFilterCombos();
		runFilter();
	}

	public IDList getIDList() {
		return idlist;
	}

	public void setResultFileManager(ResultFileManagerEx manager) {
		table.setResultFileManager(manager);
	}

	public ResultFileManagerEx getResultFileManager() {
		return table.getResultFileManager();
	}

	protected void initialize(int type) {
		setLayout(new GridLayout(1, false));
		filterPanel = new FilteringPanel(this, SWT.NONE);
		filterPanel.setLayoutData(new GridData(SWT.FILL, SWT.BEGINNING, true, false));
		table = new DataTable(this, SWT.MULTI, type);
		table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
	}

	protected void configureFilterPanel() {
		SelectionListener selectionListener = new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				runFilter();
			}
			public void widgetDefaultSelected(SelectionEvent e) {
				runFilter();
			}
		};
			
		// when the filter button gets pressed, update the table
		filterPanel.getFilterButton().addSelectionListener(selectionListener);
		filterPanel.getFileNameCombo().addSelectionListener(selectionListener);
		filterPanel.getRunNameCombo().addSelectionListener(selectionListener);
		filterPanel.getExperimentNameCombo().addSelectionListener(selectionListener);
		filterPanel.getMeasurementNameCombo().addSelectionListener(selectionListener);
		filterPanel.getReplicationNameCombo().addSelectionListener(selectionListener);
		filterPanel.getModuleNameCombo().addSelectionListener(selectionListener);
		filterPanel.getNameCombo().addSelectionListener(selectionListener);
	}

	protected void updateFilterCombos() {
		filterPanel.setFilterHints(getFilterHints());
	}
	
	public FilterHints getFilterHints() {
		return new FilterHints(table.getResultFileManager(), idlist);
	}
	
	protected void runFilter() {
		// run the filter on the unfiltered IDList, and set the result to the table
		FilterParams params = getFilterParams();
		IDList filteredIDList = ScaveModelUtil.filterIDList(idlist, params, table.getResultFileManager());
		table.setIDList(filteredIDList);
		filterPanel.setFilterText(params.toString());
	}
	
	public FilterParams getFilterParams() {
		return filterPanel.getFilterParams();
	}
	
	public void setFilterParams(FilterParams params) {
		filterPanel.setFilterParams(params);
		runFilter();
	}

	/**
	 * This method initializes formToolkit	
	 * 	
	 * @return org.eclipse.ui.forms.widgets.FormToolkit	
	 */
	private FormToolkit getFormToolkit() {
		if (formToolkit == null) {
			formToolkit = new FormToolkit(Display.getCurrent());
		}
		return formToolkit;
	}
}
