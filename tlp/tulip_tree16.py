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
    for n in graph.getNodes():
        if graph.deg(n) == 1:
            viewColor[n] = tlp.Color.SpringGreen
        else:
            viewColor[n] = tlp.Color.JungleGreen
   
    for n in graph.getNodes():
        viewBorderWidth[n] = 1
    # Compute an anonymous degree property
    degree = tlp.DoubleProperty(graph)
    degreeParams = tlp.getDefaultPluginParameters("Degree")
    graph.applyDoubleAlgorithm("Degree", degree, degreeParams)
    # Map the node sizes to their degree
    
    # Compute an anonymous degree property
    degree = tlp.DoubleProperty(graph)
    degreeParams = tlp.getDefaultPluginParameters("Degree")
    graph.applyDoubleAlgorithm("Degree", degree, degreeParams)
    baseSize = tlp.Size(1,1,1)/13
    for n in graph.getNodes():
        viewSize[n] = baseSize * (graph.deg(n) + 1)
    sizeMappingParams = tlp.getDefaultPluginParameters("Size Mapping", graph)
    graph.applySizeAlgorithm("Size Mapping", viewSize, sizeMappingParams)
    # Create a Node Link Diagram view without displaying it
    nodeLinkView = tlpgui.createView("Node Link Diagram view", graph, {}, False)
    renderingParams = nodeLinkView.getRenderingParameters()
    # Set border colors values
 #   viewBorderColor.setAllNodeValue(tlp.Color.Black)
 #   viewLabelColor.setAllNodeValue(tlp.Color.Blue)
  #  viewLabelBorderColor.setAllNodeValue(tlp.Color.Blue)
    # Add a border to nodes/edges
   # viewBorderWidth.setAllNodeValue(0.0002)
   # viewBorderWidth.setAllEdgeValue(0.0002)
    # Sets nodes shapes to circle
    viewShape.setAllNodeValue(tlp.NodeShape.Circle)
    # Activate the ordered rendering mode
   # renderingParams.setElementOrdered(True)
   # renderingParams.setElementOrderingProperty(renderingOrderingProp)
    # Activate the "no labels overlaps" mode
    renderingParams.setLabelsDensity(0)
    renderingParams.setMinSizeOfLabel(4)
    renderingParams.setEdge3D(True)
    nodeLinkView.setRenderingParameters(renderingParams)
 #View Center
    #updateVisualization(centerViews = True)
    #nodeLinkView.zoomFactor(1.08)
    #nodeLinkView.saveSnapshot("/local/tree16_view1.png", 2048, 2048)
 #View Zoomed 01
    updateVisualization(centerViews = True)
    nodeLinkView.zoomXY(18, -5, -60)
    nodeLinkView.saveSnapshot("/local/tree16.png", 2048, 2048)

