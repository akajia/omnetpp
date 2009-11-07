<#-- template include for network.ned.ftl -->
<#if generateNodeTypeDecl>
module ${nodeTypeName} {
    parameters:
        @display("i=misc/node_vs");
    gates:
        inout g[];
}
</#if>

<#if generateChannelTypeDecl && channelTypeName!="">
channel ${channelTypeName} extends ned.DatarateChannel {
    parameters:
        int cost = default(0);
}
</#if>

<#-- TODO: generateCoordinates -->
<#if parametricNED>
network ${nedTypeName}
{
    parameters:
        int n = default(${nodes});
    submodules:
        node[n]: ${nodeTypeName};
    connections:
        for i=0..n-1 {
            node[i].g++ <-->${channelSpec} node[(i+1)%n].g++;
        }
}
<#else>
network ${nedTypeName}
{
    submodules:
<#list 0..nodes-1 as i>
        node${i}: ${nodeTypeName};
</#list>
    connections:
<#list 0..nodes-1 as i>
        node${i}.g++ <-->${channelSpec} node${(i+1)%nodes}.g++;
</#list>
}
</#if>
