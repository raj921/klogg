/*
 * Copyright (C) 2009, 2010, 2011, 2013 Nicolas Bonnefon
 * and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRAWLERWIDGET_H
#define CRAWLERWIDGET_H

#include <QSplitter>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "logmainview.h"
#include "filteredview.h"
#include "data/logdata.h"
#include "data/logfiltereddata.h"
#include "viewinterface.h"

class InfoLine;
class QuickFindPattern;
class SavedSearches;
class Overview;
class QStandardItemModel;
class OverviewWidget;

// Implements the central widget of the application.
// It includes both windows, the search line, the info
// lines and various buttons.
class CrawlerWidget : public QSplitter,
    public QuickFindMuxSelectorInterface, public ViewInterface
{
  Q_OBJECT

  public:
    CrawlerWidget( QWidget *parent=0 );

    // Get the line number of the first line displayed.
    int getTopLine() const;
    // Get the selected text as a string (from the main window)
    QString getSelectedText() const;

    // Display the QFB at the bottom, remembering where the focus was
    void displayQuickFindBar( QuickFindMux::QFDirection direction );

    // Instructs the widget to select all the text in the window the user
    // is interacting with
    void selectAll();

  public slots:
    // Stop the asynchoronous loading of the file if one is in progress
    // The file is identified by the view attached to it.
    void stopLoading();
    // Reload the displayed file
    void reload();

  protected:
    void keyPressEvent( QKeyEvent* keyEvent );

    // Implementation of the ViewInterface functions
    virtual void doSetData(
            std::shared_ptr<LogData> log_data,
            std::shared_ptr<LogFilteredData> filtered_data );
    virtual void doSetQuickFindPattern(
            std::shared_ptr<QuickFindPattern> qfp );
    virtual void doSetSavedSearches(
            std::shared_ptr<SavedSearches> saved_searches );

    // Implementation of the mux selector interface
    // (for dispatching QuickFind to the right widget)
    virtual SearchableWidgetInterface* doGetActiveSearchable() const;
    virtual std::vector<QObject*> doGetAllSearchables() const;

  signals:
    // Sent to signal the client load has progressed,
    // passing the completion percentage.
    void loadingProgressed( int progress );
    // Sent to the client when the loading has finished
    // weither succesfull or not.
    void loadingFinished( bool success );
    // Sent when follow mode is enabled/disabled
    void followSet( bool checked );
    // Sent up to the MainWindow to disable the follow mode
    void followDisabled();
    // Sent up when the current line number is updated
    void updateLineNumber( int line );

  private slots:
    // Instructs the widget to start a search using the current search line.
    void startNewSearch();
    // Stop the currently ongoing search (if one exists)
    void stopSearch();
    // Instructs the widget to reconfigure itself because Config() has changed.
    void applyConfiguration();
    // Called when new data must be displayed in the filtered window.
    void updateFilteredView( int nbMatches, int progress );
    // Called when a new line has been selected in the filtered view,
    // to instruct the main view to jump to the matching line.
    void jumpToMatchingLine( int filteredLineNb );
    // Mark a line that has been clicked on the main (top) view.
    void markLineFromMain( qint64 line );
    // Mark a line that has been clicked on the filtered (bottom) view.
    void markLineFromFiltered( qint64 line );

    void loadingFinishedHandler( bool success );
    // Manages the info lines to inform the user the file has changed.
    void fileChangedHandler( LogData::MonitoredFileStatus );

    void hideQuickFindBar();

    // Instructs the widget to change the pattern in the QuickFind widget
    // and confirm it.
    void changeQFPattern( const QString& newPattern );

    void searchForward();
    void searchBackward();

    // Called when the checkbox for search auto-refresh is changed
    void searchRefreshChangedHandler( int state );

    // Called when the text on the search line is modified
    void searchTextChangeHandler();

    // Called when the user change the visibility combobox
    void changeFilteredViewVisibility( int index );

    // Called when the user add the string to the search
    void addToSearch( const QString& string );

    // Called when a match is hovered on in the filtered view
    void mouseHoveredOverMatch( qint64 line );

  private:
    // State machine holding the state of the search, used to allow/disallow
    // auto-refresh and inform the user via the info line.
    class SearchState {
      public:
        enum State {
            NoSearch,
            Static,
            Autorefreshing,
            FileTruncated,
        };

        SearchState() { state_ = NoSearch; autoRefreshRequested_ = false; }

        // Reset the state (no search active)
        void resetState();
        // The user changed auto-refresh request
        void setAutorefresh( bool refresh );
        // The file has been truncated (stops auto-refresh)
        void truncateFile();
        // The expression has been changed (stops auto-refresh)
        void changeExpression();
        // The search has been stopped (stops auto-refresh)
        void stopSearch();
        // The search has been started (enable auto-refresh)
        void startSearch();

        // Get the state in order to display the proper message
        State getState() const { return state_; }
        // Is auto-refresh allowed
        bool isAutorefreshAllowed() const
            { return ( state_ == Autorefreshing ); }

      private:
        State state_;
        bool autoRefreshRequested_;
    };

    // Private functions
    void setup();
    void replaceCurrentSearch( const QString& searchText );
    void updateSearchCombo();
    AbstractLogView* activeView() const;
    void printSearchInfoMessage( int nbMatches = 0 );

    // Palette for error notification (yellow background)
    static const QPalette errorPalette;

    LogMainView*    logMainView;
    QWidget*        bottomWindow;
    QLabel*         searchLabel;
    QComboBox*      searchLineEdit;
    QToolButton*    searchButton;
    QToolButton*    stopButton;
    FilteredView*   filteredView;
    QComboBox*      visibilityBox;
    InfoLine*       searchInfoLine;
    QCheckBox*      ignoreCaseCheck;
    QCheckBox*      searchRefreshCheck;
    OverviewWidget* overviewWidget_;

    QVBoxLayout*    bottomMainLayout;
    QHBoxLayout*    searchLineLayout;
    QHBoxLayout*    searchInfoLineLayout;

    // Default palette to be remembered
    QPalette        searchInfoLineDefaultPalette;

    std::shared_ptr<SavedSearches> savedSearches_;

    // Reference to the QuickFind Pattern (not owned)
    std::shared_ptr<QuickFindPattern> quickFindPattern_;

    LogData*        logData_;
    LogFilteredData* logFilteredData_;

    qint64          logFileSize_;

    QWidget*        qfSavedFocus_;

    // Search state (for auto-refresh and truncation)
    SearchState     searchState_;

    // Matches overview
    Overview*       overview_;

    // Model for the visibility selector
    QStandardItemModel* visibilityModel_;
};

#endif
