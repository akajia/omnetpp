package org.omnetpp.common.eventlog;

import org.eclipse.ui.views.properties.IPropertySource;
import org.eclipse.ui.views.properties.IPropertySourceProvider;
import org.omnetpp.common.properties.PropertySource;

public class EventLogConnectionPropertySource extends PropertySource {
    private static final String PROPERTY_SOURCE_MODULE      = "SourceModule";
    private static final String PROPERTY_SOURCE_GATE        = "SourceGate";
    private static final String PROPERTY_DESTINATION_MODULE = "DestinationModule";
    private static final String PROPERTY_DESTINATION_GATE   = "DestinationGate";
    private static final String PROPERTY_DISPLAY_STRING     = "DisplayString";

    private IPropertySourceProvider propertySourceProvider;
    private EventLogConnection eventLogConnection;

    public EventLogConnectionPropertySource(IPropertySourceProvider propertySourceProvider, EventLogConnection eventLogConnection) {
        this.propertySourceProvider = propertySourceProvider;
        this.eventLogConnection = eventLogConnection;
    }

    @org.omnetpp.common.properties.Property(id = PROPERTY_SOURCE_MODULE, category = "Source")
    public IPropertySource getSourceModule() { return propertySourceProvider.getPropertySource(eventLogConnection.getSourceModule()); }

    @org.omnetpp.common.properties.Property(id = PROPERTY_SOURCE_GATE, category = "Source")
    public IPropertySource getSourceGate() { return propertySourceProvider.getPropertySource(eventLogConnection.getSourceGate()); }

    @org.omnetpp.common.properties.Property(id = PROPERTY_DESTINATION_MODULE, category = "Destination")
    public IPropertySource getDestinationModule() { return propertySourceProvider.getPropertySource(eventLogConnection.getDestinationModule()); }

    @org.omnetpp.common.properties.Property(id = PROPERTY_DESTINATION_GATE, category = "Destination")
    public IPropertySource getDestinationGate() { return propertySourceProvider.getPropertySource(eventLogConnection.getDestinationGate()); }

    @org.omnetpp.common.properties.Property(id = PROPERTY_DISPLAY_STRING)
    public String getDisplayString() { return eventLogConnection.getDisplayString() == null ? "" : eventLogConnection.getDisplayString().toString(); }
}
