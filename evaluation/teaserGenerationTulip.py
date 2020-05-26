# Powered by Python 2.7
# To cancel the modifications performed by the script
# on the current graph, click on the undo button.
# Some useful keyboard shortcuts:
#   * Ctrl + D: comment selected lines.
#   * Ctrl + Shift + D: uncomment selected lines.
#   * Ctrl + I: indent selected lines.
#   * Ctrl + Shift + I: unindent selected lines.
#   * Ctrl + Return: run script.
#   * Ctrl + F: find selected text.
#   * Ctrl + R: replace selected text.
#   * Ctrl + Space: show auto-completion dialog.
from tulip import tlp
# The updateVisualization(centerViews = True) function can be called
# during script execution to update the opened views
# The pauseScript() function can be called to pause the script execution.
# To resume the script execution, you will have to click on the
# "Run script " button.
# The runGraphScript(scriptFile, graph) function can be called to launch
# another edited script on a tlp.Graph object.
# The scriptFile parameter defines the script name to call
# (in the form [a-zA-Z0-9_]+.py)
# The main(graph) function must be defined
# to run the script on the current graph
def main(graph):
    
    teaser_graph = [
                    "/Users/mweier2m/dev/owlspringembedder/results/teaser1%s.%s",
                    "/Users/mweier2m/dev/owlspringembedder/results/teaser100%s.%s",
                    "/Users/mweier2m/dev/owlspringembedder/results/teaser2000%s.%s",
                    "/Users/mweier2m/dev/owlspringembedder/results/teaser12000%s.%s"]

    
    # Create a Node Link Diagram view without displaying it
    nodeLinkView = tlpgui.createView("Node Link Diagram view", graph, {}, False)
    nodeLinkView.zoomFactor (1.05)
    nodeLinkView.resize(2048,2048);
 
    state = nodeLinkView.state();    
    print(state)
    
    for fnGraph in teaser_graph:
        print(fnGraph % ("","tlp"))
        graph = tlp.loadGraph(fnGraph % ("", "tlp"))
    
        viewBorderColor = graph['viewBorderColor']
        viewBorderWidth = graph['viewBorderWidth']
        viewColor = graph['viewColor']
        viewFont = graph['viewFont']
        viewFontSize = graph['viewFontSize']
        viewIcon = graph['viewIcon']
        viewLabel = graph['viewLabel']
        viewLabelBorderColor = graph['viewLabelBorderColor']
        viewLabelBorderWidth = graph['viewLabelBorderWidth']
        viewLabelColor = graph['viewLabelColor']
        viewLabelPosition = graph['viewLabelPosition']
        viewLayout = graph['viewLayout']
        viewMetric = graph['viewMetric']
        viewRotation = graph['viewRotation']
        viewSelection = graph['viewSelection']
        viewShape = graph['viewShape']
        viewSize = graph['viewSize']
        viewSrcAnchorShape = graph['viewSrcAnchorShape']
        viewSrcAnchorSize = graph['viewSrcAnchorSize']
        viewTexture = graph['viewTexture']
        viewTgtAnchorShape = graph['viewTgtAnchorShape']
        viewTgtAnchorSize = graph['viewTgtAnchorSize']
    
        #for n in graph.getNodes():
        #    print(n)
        viewColor = graph.getColorProperty("viewColor")
    
        # Constant size
       
        for n in graph.getNodes():
            if graph.deg(n) > 3:
                viewColor[n] = tlp.Color.Azure
            else:
                viewColor[n] = tlp.Color.Blue
        
        # Compute an anonymous degree property
        degree = tlp.DoubleProperty(graph)
        degreeParams = tlp.getDefaultPluginParameters("Degree")
        degreeParams['norm'] = True
        graph.applyDoubleAlgorithm("Degree", degree, degreeParams)
        
        # Map the node sizes to their degree
        #sizeMappingParams = tlp.getDefaultPluginParameters("Size Mapping", graph)
        #sizeMappingParams["property"] = degree
        #sizeMappingParams["min size"] = 0.4
        #sizeMappingParams["max size"] = 0.6
        #graph.applySizeAlgorithm("Size Mapping", viewSize, sizeMappingParams)
        
        #####
        baseSize = tlp.Size(0.4,0.4,0.4)
        
        for n in graph.getNodes():
            viewSize[n] = baseSize * (degree[n] + 1)
            
       
   
        renderingParams = nodeLinkView.getRenderingParameters()
        print(renderingParams)
        # Set border colors values
        # viewBorderColor.setAllNodeValue(tlp.Color.Black)
        # viewLabelColor.setAllNodeValue(tlp.Color.Blue)
        # viewLabelBorderColor.setAllNodeValue(tlp.Color.Blue)
        
        # Add a border to nodes/edges
        # viewBorderWidth.setAllNodeValue(0.0002)
        # viewBorderWidth.setAllEdgeValue(0.0002)
        
        # Sets nodes shapes to circle
        viewShape.setAllNodeValue(tlp.NodeShape.Circle)    
        
        # Activate the ordered rendering mode
        # renderingParams.setElementOrdered(True)
        # renderingParams.setElementOrderingProperty(renderingOrderingProp)
        
        # Activate the "no labels overlaps" mode
        #renderingParams.setLabelsDensity(0)
        #renderingParams.setMinSizeOfLabel(4)
        renderingParams.setEdge3D(True)
        nodeLinkView.setRenderingParameters(renderingParams)
    
        #Create First view           
        nodeLinkView.setGraph(graph) 
        nodeLinkView.setState(state);
        
        nodeLinkView.saveSnapshot(fnGraph % ("full","png"), 2048, 2048)
            

        #First Zoom View
        nodeLinkView.zoomXY(20, 300, 50);
        nodeLinkView.saveSnapshot(fnGraph % ("zoom1","png"), 2048, 2048)
        nodeLinkView.setState(state);
        nodeLinkView.zoomXY(20, 400, 350);        
        nodeLinkView.saveSnapshot(fnGraph % ("zoom2","png"), 2048, 2048)
        
