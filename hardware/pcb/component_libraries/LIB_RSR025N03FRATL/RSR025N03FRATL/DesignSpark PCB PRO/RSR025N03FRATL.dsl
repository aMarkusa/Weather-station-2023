SamacSys ECAD Model
646417/537576/2.49/3/3/MOSFET N-Channel

DESIGNSPARK_INTERMEDIATE_ASCII

(asciiHeader
	(fileUnits MM)
)
(library Library_1
	(padStyleDef "r115_60"
		(holeDiam 0)
		(padShape (layerNumRef 1) (padShapeType Rect)  (shapeWidth 0.6) (shapeHeight 1.15))
		(padShape (layerNumRef 16) (padShapeType Ellipse)  (shapeWidth 0) (shapeHeight 0))
	)
	(textStyleDef "Default"
		(font
			(fontType Stroke)
			(fontFace "Helvetica")
			(fontHeight 50 mils)
			(strokeWidth 5 mils)
		)
	)
	(patternDef "SOT95P280X100-3N" (originalName "SOT95P280X100-3N")
		(multiLayer
			(pad (padNum 1) (padStyleRef r115_60) (pt -1.25, 0.95) (rotation 90))
			(pad (padNum 2) (padStyleRef r115_60) (pt -1.25, -0.95) (rotation 90))
			(pad (padNum 3) (padStyleRef r115_60) (pt 1.25, 0) (rotation 90))
		)
		(layerContents (layerNumRef 18)
			(attr "RefDes" "RefDes" (pt 0, 0) (textStyleRef "Default") (isVisible True))
		)
		(layerContents (layerNumRef 30)
			(line (pt -2.075 1.7) (pt 2.075 1.7) (width 0.05))
		)
		(layerContents (layerNumRef 30)
			(line (pt 2.075 1.7) (pt 2.075 -1.7) (width 0.05))
		)
		(layerContents (layerNumRef 30)
			(line (pt 2.075 -1.7) (pt -2.075 -1.7) (width 0.05))
		)
		(layerContents (layerNumRef 30)
			(line (pt -2.075 -1.7) (pt -2.075 1.7) (width 0.05))
		)
		(layerContents (layerNumRef 28)
			(line (pt -0.8 1.45) (pt 0.8 1.45) (width 0.1))
		)
		(layerContents (layerNumRef 28)
			(line (pt 0.8 1.45) (pt 0.8 -1.45) (width 0.1))
		)
		(layerContents (layerNumRef 28)
			(line (pt 0.8 -1.45) (pt -0.8 -1.45) (width 0.1))
		)
		(layerContents (layerNumRef 28)
			(line (pt -0.8 -1.45) (pt -0.8 1.45) (width 0.1))
		)
		(layerContents (layerNumRef 28)
			(line (pt -0.8 0.5) (pt 0.15 1.45) (width 0.1))
		)
		(layerContents (layerNumRef 18)
			(line (pt -0.325 1.45) (pt 0.325 1.45) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt 0.325 1.45) (pt 0.325 -1.45) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt 0.325 -1.45) (pt -0.325 -1.45) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt -0.325 -1.45) (pt -0.325 1.45) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt -1.825 1.5) (pt -0.675 1.5) (width 0.2))
		)
	)
	(symbolDef "RSR025N03FRATL" (originalName "RSR025N03FRATL")

		(pin (pinNum 1) (pt 0 mils 0 mils) (rotation 0) (pinLength 100 mils) (pinDisplay (dispPinName false)) (pinName (text (pt 0 mils -45 mils) (rotation 0]) (justify "UpperLeft") (textStyleRef "Default"))
		))
		(pin (pinNum 2) (pt 300 mils 400 mils) (rotation 270) (pinLength 100 mils) (pinDisplay (dispPinName false)) (pinName (text (pt 305 mils 400 mils) (rotation 90]) (justify "UpperLeft") (textStyleRef "Default"))
		))
		(pin (pinNum 3) (pt 300 mils -200 mils) (rotation 90) (pinLength 100 mils) (pinDisplay (dispPinName false)) (pinName (text (pt 305 mils -200 mils) (rotation 90]) (justify "LowerLeft") (textStyleRef "Default"))
		))
		(line (pt 300 mils 100 mils) (pt 300 mils -100 mils) (width 6 mils))
		(line (pt 300 mils 200 mils) (pt 300 mils 300 mils) (width 6 mils))
		(line (pt 100 mils 0 mils) (pt 200 mils 0 mils) (width 6 mils))
		(line (pt 200 mils 0 mils) (pt 200 mils 200 mils) (width 6 mils))
		(line (pt 300 mils 100 mils) (pt 230 mils 100 mils) (width 6 mils))
		(line (pt 300 mils 200 mils) (pt 230 mils 200 mils) (width 6 mils))
		(line (pt 230 mils 0 mils) (pt 300 mils 0 mils) (width 6 mils))
		(line (pt 230 mils 220 mils) (pt 230 mils 180 mils) (width 6 mils))
		(line (pt 230 mils -20 mils) (pt 230 mils 20 mils) (width 6 mils))
		(line (pt 230 mils 80 mils) (pt 230 mils 120 mils) (width 6 mils))
		(arc (pt 250 mils 100 mils) (radius 150 mils) (startAngle 0) (sweepAngle 360) (width 10  mils))
		(poly (pt 230 mils 100 mils) (pt 270 mils 120 mils) (pt 270 mils 80 mils) (pt 230 mils 100 mils) (width 10  mils))
		(attr "RefDes" "RefDes" (pt 450 mils 150 mils) (justify Left) (isVisible True) (textStyleRef "Default"))

	)
	(compDef "RSR025N03FRATL" (originalName "RSR025N03FRATL") (compHeader (numPins 3) (numParts 1) (refDesPrefix Q)
		)
		(compPin "1" (pinName "G") (partNum 1) (symPinNum 1) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "2" (pinName "S") (partNum 1) (symPinNum 3) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(compPin "3" (pinName "D") (partNum 1) (symPinNum 2) (gateEq 0) (pinEq 0) (pinType Bidirectional))
		(attachedSymbol (partNum 1) (altType Normal) (symbolName "RSR025N03FRATL"))
		(attachedPattern (patternNum 1) (patternName "SOT95P280X100-3N")
			(numPads 3)
			(padPinMap
				(padNum 1) (compPinRef "1")
				(padNum 2) (compPinRef "2")
				(padNum 3) (compPinRef "3")
			)
		)
		(attr "Manufacturer_Name" "ROHM Semiconductor")
		(attr "Manufacturer_Part_Number" "RSR025N03FRATL")
		(attr "Mouser Part Number" "")
		(attr "Mouser Price/Stock" "")
		(attr "Arrow Part Number" "")
		(attr "Arrow Price/Stock" "")
		(attr "Mouser Testing Part Number" "")
		(attr "Mouser Testing Price/Stock" "")
		(attr "Description" "ROHM - RSR025N03FRATL - Power MOSFET, N Channel, 30 V, 2.5 A, 0.05 ohm, TSMT, Surface Mount")
		(attr "Datasheet Link" "https://componentsearchengine.com/Datasheets/2/RSR025N03FRATL.pdf")
		(attr "Height" "1 mm")
	)

)
