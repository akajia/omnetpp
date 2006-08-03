package org.omnetpp.scave2.model;

import static org.omnetpp.scave2.model.RunAttribute.EXPERIMENT;
import static org.omnetpp.scave2.model.RunAttribute.MEASUREMENT;
import static org.omnetpp.scave2.model.RunAttribute.REPLICATION;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.eclipse.core.runtime.Assert;
import org.eclipse.emf.common.command.Command;
import org.eclipse.emf.common.util.EList;
import org.eclipse.emf.common.util.TreeIterator;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.resource.Resource;
import org.eclipse.emf.edit.command.AddCommand;
import org.eclipse.emf.edit.command.SetCommand;
import org.eclipse.emf.edit.domain.EditingDomain;
import org.omnetpp.scave.engine.FileRunList;
import org.omnetpp.scave.engine.IDList;
import org.omnetpp.scave.engine.ResultFile;
import org.omnetpp.scave.engine.ResultFileList;
import org.omnetpp.scave.engine.ResultItem;
import org.omnetpp.scave.engine.Run;
import org.omnetpp.scave.engine.RunList;
import org.omnetpp.scave.engine.StringMap;
import org.omnetpp.scave.engine.ResultFileManager;
import org.omnetpp.scave.model.Add;
import org.omnetpp.scave.model.Analysis;
import org.omnetpp.scave.model.Chart;
import org.omnetpp.scave.model.Dataset;
import org.omnetpp.scave.model.DatasetType;
import org.omnetpp.scave.model.Property;
import org.omnetpp.scave.model.ScaveModelFactory;
import org.omnetpp.scave.model.ScaveModelPackage;

/**
 * A collection of static methods to manipulate model objects
 * @author andras
 */
public class ScaveModelUtil {
	
	public enum RunIdKind {
		FILE_RUN,
		EXPERIMENT_MEASUREMENT_REPLICATION
	}
	
	public static Dataset createDataset(String name, DatasetType type, FilterParams params) {
		Dataset dataset = ScaveModelFactory.eINSTANCE.createDataset();
		dataset.setName(name);
		dataset.setType(type);
		dataset.getItems().add(createAdd(params));
		return dataset;
	}
	
	public static Dataset createDataset(String name, DatasetType type, ResultItem[] items, RunIdKind id) {
		Dataset dataset = ScaveModelFactory.eINSTANCE.createDataset();
		dataset.setName(name);
		dataset.setType(type);
		dataset.getItems().addAll(createAdds(items, id));
		return dataset;
	}
	
	public static Chart createChart(String name) {
		Chart chart = ScaveModelFactory.eINSTANCE.createChart();
		chart.setName(name);
		return chart;
	}
	
	public static Add createAdd(FilterParams params) {
		Add add = ScaveModelFactory.eINSTANCE.createAdd();
		add.setFilenamePattern(params.getFileNamePattern());
		add.setRunNamePattern(params.getRunNamePattern());
		add.setExperimentNamePattern(params.getExperimentNamePattern());
		add.setMeasurementNamePattern(params.getMeasurementNamePattern());
		add.setReplicationNamePattern(params.getReplicationNamePattern());
		add.setModuleNamePattern(params.getModuleNamePattern());
		add.setNamePattern(params.getDataNamePattern());
		return add;
	}
	
	public static Collection<Add> createAdds(ResultItem[] items, RunIdKind id) {
		List<Add> adds = new ArrayList<Add>(items.length);
		for (ResultItem item : items)
			adds.add(createAdd(item, id));
		return adds;
	}
	
	public static Add createAdd(ResultItem item, RunIdKind id) {
		Add add = ScaveModelFactory.eINSTANCE.createAdd();
		ResultFile file = item.getFileRun().getFile();
		Run run = item.getFileRun().getRun();
		if (id == RunIdKind.FILE_RUN) {
			add.setFilenamePattern(file.getFilePath());
			add.setRunNamePattern(run.getRunName());
		}
		else if (id == RunIdKind.EXPERIMENT_MEASUREMENT_REPLICATION) {
			add.setExperimentNamePattern(run.getAttribute(RunAttribute.EXPERIMENT));
			add.setMeasurementNamePattern(run.getAttribute(RunAttribute.MEASUREMENT));
			add.setReplicationNamePattern(run.getAttribute(RunAttribute.REPLICATION));
		}
		add.setModuleNamePattern(item.getModuleName());
		add.setNamePattern(item.getName());
		return add;
	}
	
	public static Dataset findEnclosingDataset(Chart chart) {
		EObject parent = chart.eContainer();
		while (parent != null && !(parent instanceof Dataset))
			parent = parent.eContainer();
		return (Dataset)parent;
	}

	/**
	 * Returns the analysis node of the specified resource.
	 * It is assumed that the resource contains exactly one analysis node as 
	 * content.
	 */
	public static Analysis findAnalysis(Resource resource) {
		EList contents = resource.getContents();
		Assert.isTrue(contents.size() == 1 && contents.get(0) instanceof Analysis, "Analysis object expected");
		return (Analysis)contents.get(0);
	}
	
	/**
	 * Returns the datasets in the resource having the specified type.
	 */
	public static List<Dataset> findDatasets(Resource resource, DatasetType type) {
		List<Dataset> result = new ArrayList<Dataset>();
		Analysis analysis = findAnalysis(resource);
		if (analysis.getDatasets() != null) {
			for (Object object : analysis.getDatasets().getDatasets()) {
				Dataset dataset = (Dataset)object;
				if (dataset.getType().equals(type))
					result.add((Dataset)dataset);
			}
		}
		return result;
	}
	
	public static <T extends EObject> T findEnclosingObject(EObject object, Class<T> type) {
		while (object != null && !type.isInstance(object))
			object = object.eContainer();
		return (T)object;
	}
	
	/**
	 * Returns all object in the container having the specified type. 
	 */
	public static <T extends EObject> List<T> findObjects(EObject container, Class<T> type) {
		ArrayList<T> objects = new ArrayList<T>();
		for (TreeIterator iterator = container.eAllContents(); iterator.hasNext(); ) {
			Object object = iterator.next();
			if (type.isInstance(object))
				objects.add((T)object);
		}
		return objects;
 	}
	
	/**
	 * Returns all objects in the resource having the specified type.
	 */
	public static <T extends EObject> List<T> findObjects(Resource resource, Class<T> type) {
		ArrayList<T> objects = new ArrayList<T>();
		for (TreeIterator iterator = resource.getAllContents(); iterator.hasNext(); ) {
			Object object = iterator.next();
			if (type.isInstance(object))
				objects.add((T)object);
		}
		return objects;
	}
	
	public static Property getChartProperty(Chart chart, String propertyName) {
		for (Object object : chart.getProperties()) {
			Property property = (Property)object;
			if (property.getName().equals(propertyName))
				return property;
		}
		return null;
	}
	
	public static void setChartProperty(EditingDomain ed, Chart chart, String propertyName, String propertyValue) {
		Property property = getChartProperty(chart, propertyName);
		Command command;
		if (property == null) {
			property = ScaveModelFactory.eINSTANCE.createProperty();
			property.setName(propertyName);
			property.setValue(propertyValue);
			command = AddCommand.create(
						ed,
						chart,
						ScaveModelPackage.eINSTANCE.getChart_Properties(),
						property);
		}
		else {
			command = SetCommand.create(
						ed,
						property,
						ScaveModelPackage.eINSTANCE.getProperty_Value(),
						propertyValue);
		}
		ed.getCommandStack().execute(command);
	}
	
	public static IDList getAllIDs(ResultFileManager manager, DatasetType type) {
		IDList idlist = null;
		switch (type.getValue()) {
		case DatasetType.SCALAR: 	idlist = manager.getAllScalars();
		case DatasetType.VECTOR:	idlist = manager.getAllVectors();
		case DatasetType.HISTOGRAM:	idlist = null; // TODO
		}
		return idlist;
	}
	
	public static IDList filterIDList(IDList idlist, FilterParams params, ResultFileManager manager) {
		ResultFileList fileList = params.getFileNamePattern().length() > 0 ?
				manager.filterFileList(manager.getFiles(), params.getFileNamePattern()) : null;
		StringMap attrs = new StringMap();
		addAttribute(attrs, EXPERIMENT, params.getExperimentNamePattern());
		addAttribute(attrs, MEASUREMENT, params.getMeasurementNamePattern());
		addAttribute(attrs, REPLICATION, params.getReplicationNamePattern());
		String runNamePattern = params.getRunNamePattern().length() > 0 ? params.getRunNamePattern() : "*";
		RunList runList = params.getRunNamePattern().length() > 0 || attrs.size() > 0 ?
				manager.filterRunList(manager.getRuns(), runNamePattern, attrs) : null;
		FileRunList fileRunFilter = manager.getFileRuns(fileList, runList);
		IDList filteredIDList = manager.filterIDList(idlist,
				fileRunFilter, params.getModuleNamePattern(), params.getDataNamePattern());
		return filteredIDList;
	}
	
	private static void addAttribute(StringMap attrs, String name, String value) {
		if (value != null && value.length() > 0)
			attrs.set(name, value);
	}
}
