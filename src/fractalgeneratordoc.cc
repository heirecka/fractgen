/* ==========================================================================
 * ====                   FRACTAL GRAPHICS GENERATOR                     ====
 * ==========================================================================
 *
 * Copyright (C) 2003-2020 by Thomas Dreibholz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: dreibh@iem.uni-due.de
 */

#include "fractalgeneratordoc.h"
#include "fractalgenerator.h"
#include "fractalgenerator.h"
#include "fractalgeneratorview.h"

#include <QtWidgets/QMessageBox>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDataStream>


// ###### Constructor #######################################################
FractalGeneratorDoc::FractalGeneratorDoc(QWidget* parent, FractalGeneratorView* view)
   : QObject(parent)
{
   View        = view;
   Application = static_cast<FractalGeneratorApp*>(parent);
}


// ###### Destructor ########################################################
FractalGeneratorDoc::~FractalGeneratorDoc()
{
   closeDocument();
}


// ###### Close document ####################################################
void FractalGeneratorDoc::closeDocument()
{
}


// ###### New document ######################################################
void FractalGeneratorDoc::newDocument()
{
   Modified = false;
   setFileName(tr("Unnamed.fsf"));
}


// ###### Open document #####################################################
bool FractalGeneratorDoc::openDocument(const QString& fileName)
{
   // ====== Open file ======================================================
   QFile file(fileName);
   if(!file.open(QIODevice::ReadOnly)) {
      return(false);
   }

   // ====== Parse XML document =============================================
   QDomDocument doc(QStringLiteral("XMLFractalSave"));
   QString      errorText;
   int          line, column;
   if(!doc.setContent(&file, false, &errorText, &line, &column)) {
      QMessageBox::warning(Application, tr("Open File Failure"),
                           errorText + tr(" in line ") + QString().setNum(line) + tr(", column ") + QString().setNum(column));
      return(false);
   }

   // ====== Get fractal configuration ======================================
   // ------ Get algorithm --------------------------------------------------
   const QDomElement algorithmNameField = doc.elementsByTagName(QStringLiteral("AlgorithmName")).item(0).toElement();
   const QString     algorithmName      = algorithmNameField.firstChild().toText().data();
   unsigned int      algorithmID        = 0;
   FractalAlgorithmInterface* fractalAlgorithm;
   while( (fractalAlgorithm = FractalAlgorithmInterface::getAlgorithm(algorithmID)) != NULL ) {
      if(QString::fromLocal8Bit(fractalAlgorithm->getIdentifier()) == algorithmName) {
         break;
      }
      algorithmID++;
   }
   if(fractalAlgorithm == NULL) {
      QMessageBox::warning(Application, tr("Open File Failure"),
                           tr("Invalid AlgorithmName entry:") + algorithmName);
      return(false);
   }

   // ------ Get color scheme -----------------------------------------------
   const QDomElement colorSchemeField = doc.elementsByTagName(QStringLiteral("ColorSchemeName")).item(0).toElement();
   const QString     colorSchemeName  = colorSchemeField.firstChild().toText().data();
   unsigned int      colorSchemeID    = 0;
   ColorSchemeInterface* colorScheme;
   while((colorScheme = ColorSchemeInterface::getColorScheme(colorSchemeID))) {
      if(QString::fromLocal8Bit(colorScheme->getIdentifier()) == colorSchemeName) {
        break;
     }
     colorSchemeID++;
   }
   if(colorScheme == NULL) {
      QMessageBox::warning(Application, tr("Open File Failure"),
                           tr("Invalid ColorSchemeName entry:") + colorSchemeName);
      return(false);
   }

   // ------ Get C1 and C2 --------------------------------------------------
   const double c1real = doc.elementsByTagName(QStringLiteral("C1Real")).item(0).firstChild().toText().data().toDouble();
   const double c1imag = doc.elementsByTagName(QStringLiteral("C1Imag")).item(0).firstChild().toText().data().toDouble();
   const double c2real = doc.elementsByTagName(QStringLiteral("C2Real")).item(0).firstChild().toText().data().toDouble();
   const double c2imag = doc.elementsByTagName(QStringLiteral("C2Imag")).item(0).firstChild().toText().data().toDouble();
   const std::complex<double> C1(c1real, c1imag);
   const std::complex<double> C2(c2real, c2imag);
   View->changeC1C2(C1, C2);

   // ------ Activate settings ----------------------------------------------
   View->changeAlgorithm(algorithmID);
   View->changeColorScheme(colorSchemeID);
   View->getAlgorithm()->configure(View->getSizeX(), View->getSizeY(),
                                   C1, C2,*(View->getAlgorithm()->getMaxIterations()));

   // ------ Set user options -----------------------------------------------
   QDomElement userOptionsBranch = doc.elementsByTagName(QStringLiteral("Useroptions")).item(0).toElement();
   QDomNode    userOptionsChild  = userOptionsBranch.firstChild();
   while(!userOptionsChild.isNull()) {
      QListIterator<ConfigEntry*> iterator(*(View->getAlgorithm()->getConfigEntries()));
      const QString               name  = userOptionsChild.nodeName();
      const QString               value = userOptionsChild.firstChild().toText().data();
      bool                        found = false;
      while(iterator.hasNext()) {
         ConfigEntry* configEntry = iterator.next();
         if(configEntry->getName() == name) {
            configEntry->setValueFromString(value);
            found = true;
            break;
         }
      }
      if(!found) {
         QMessageBox::warning(Application, tr("Open File Warning"),
                              tr("Skipping unknown entry:") + name);
      }
      userOptionsChild = userOptionsChild.nextSibling();
   }

   // ------ Show new configuration -----------------------------------------
   setFileName(fileName);
   View->configChanged();

   Modified = false;
   return(true);
}


// ###### Save document #####################################################
bool FractalGeneratorDoc::saveDocument(const QString& fileName)
{
   QDomDocument doc(QStringLiteral("XMLFractalSave"));
   QDomElement root = doc.createElement(QStringLiteral("FractalV1"));
   doc.appendChild(root);

   // Create Algorithm branch
   QDomElement algorithm = doc.createElement(QStringLiteral("Algorithm"));
   root.appendChild(algorithm);

   // Algorithm Name
   QDomElement tag = doc.createElement(QStringLiteral("AlgorithmName"));
   algorithm.appendChild(tag);

   QDomText text = doc.createTextNode(QString::fromLocal8Bit(View->getAlgorithm()->getIdentifier()));
   tag.appendChild(text);

   // Algorithm C1Real
   QDomElement C = doc.createElement(QStringLiteral("C1Real"));
   algorithm.appendChild(C);
   text = doc.createTextNode(QString().setNum(View->getAlgorithm()->getC1().real(), 'e', 64));
   C.appendChild(text);

   // Algorithm C1Imag
   C = doc.createElement(QStringLiteral("C1Imag"));
   algorithm.appendChild(C);
   text = doc.createTextNode(QString().setNum(View->getAlgorithm()->getC1().imag(), 'e', 64));
   C.appendChild(text);

   // Algorithm C2Real
   C = doc.createElement(QStringLiteral("C2Real"));
   algorithm.appendChild(C);
   text = doc.createTextNode(QString().setNum(View->getAlgorithm()->getC2().real(), 'e', 64));
   C.appendChild(text);

   // Algorithm C2Imag
   C = doc.createElement(QStringLiteral("C2Imag"));
   algorithm.appendChild(C);
   text = doc.createTextNode(QString().setNum(View->getAlgorithm()->getC2().imag(), 'e', 64));
   C.appendChild(text);

   // Create UserOptions branch
   QDomElement optionRoot = doc.createElement(QStringLiteral("Useroptions"));
   algorithm.appendChild(optionRoot);

   QDomElement option;
   QListIterator<ConfigEntry*> iterator(*View->getAlgorithm()->getConfigEntries());
   while(iterator.hasNext()) {
      ConfigEntry* configEntry = iterator.next();
      option = doc.createElement(configEntry->getName());
      optionRoot.appendChild(option);
      text = doc.createTextNode(configEntry->getValueAsString());
      option.appendChild(text);
   }

   // Create ColorScheme branch
   QDomElement colorScheme = doc.createElement(QStringLiteral("ColorScheme"));
   root.appendChild(colorScheme);
   tag = doc.createElement(QStringLiteral("ColorSchemeName"));
   colorScheme.appendChild(tag);

   text = doc.createTextNode(QString::fromLocal8Bit(View->getColorScheme()->getIdentifier()));
   tag.appendChild(text);

   // Resolution
   QDomElement resolution = doc.createElement(QStringLiteral("Resolution"));
   root.appendChild( resolution );

   QString CurrentSize;
   CurrentSize += QString().setNum(View->getSizeX());
   CurrentSize += tr("*");
   CurrentSize += QString().setNum(View->getSizeY());

   text = doc.createTextNode( CurrentSize );
   resolution.appendChild(text);

   // Write XML document to file
   QString newFileName = fileName;
   if(newFileName.right(4) != QStringLiteral(".fsf")) {
      newFileName += QStringLiteral(".fsf");
   }
   QFile file(newFileName);
   file.open(QIODevice::WriteOnly);
   file.write(doc.toString().toLocal8Bit());
   setFileName(newFileName);
   Modified = false;
   return(true);
}
