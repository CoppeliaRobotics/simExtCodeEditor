#include "scintillaDlg.h"
#include "UI.h"
#include <algorithm> 
#include <QCloseEvent>
#include <QVBoxLayout>
#include <SciLexer.h>

const int fontSize=14;
const char* theFont("Courier");

CScintillaDlg::CScintillaDlg(UI *ui, QWidget* pParent)
    : QDialog(pParent),
      ui(ui)
{
    setAttribute(Qt::WA_DeleteOnClose);
    _scintillaObject=new QsciScintilla;

    QVBoxLayout *bl=new QVBoxLayout(this);
    bl->setContentsMargins(0,0,0,0);
    setLayout(bl);
    bl->addWidget(_scintillaObject);

    QsciLexerLua* lexer=new QsciLexerLua;
    _scintillaObject->setLexer(lexer);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETSTYLEBITS,(int)5);
    _scintillaObject->setTabWidth(4);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETUSETABS,(int)0);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN,(unsigned long)0,(long)48);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETMARGINWIDTHN,(unsigned long)1,(long)0);
    _scintillaObject->setFolding(QsciScintilla::BoxedTreeFoldStyle);

    connect(_scintillaObject,SIGNAL(SCN_CHARADDED(int)),this,SLOT(charAdded(int)));
    connect(_scintillaObject,SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)),this,SLOT(modified(int,int,const char*,int,int,int,int,int,int,int)));
}

CScintillaDlg::~CScintillaDlg() 
{
    // _scintillaObject is normally automatically destroyed!
}

void CScintillaDlg::setHandle(int handle)
{
    this->handle = handle;
}

void CScintillaDlg::setModal(QSemaphore *sem, QString *text, int *positionAndSize)
{
    isModal = true;
    modalData.sem = sem;
    modalData.text = text;
    modalData.positionAndSize = positionAndSize;
}

void CScintillaDlg::setAStyle(int style,QColor fore,QColor back,int size,const char *face)
{
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,(unsigned long)style,(long)fore.rgb());
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,(unsigned long)style,(long)back.rgb());
    if (size>=1)
        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETSIZE,(unsigned long)style,(long)size);
    if (face)
        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLESETFONT,(unsigned long)style,face);
}

void CScintillaDlg::setColorTheme(QColor text_col, QColor background_col, QColor selection_col, QColor comment_col, QColor number_col, QColor string_col, QColor character_col, QColor operator_col, QColor identifier_col, QColor preprocessor_col, QColor keyword1_col, QColor keyword2_col, QColor keyword3_col, QColor keyword4_col)
{
    setAStyle(QsciScintillaBase::STYLE_DEFAULT, text_col, background_col, fontSize, theFont); // set global default style
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETCARETFORE,(unsigned long)QColor(Qt::black).rgb());
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_STYLECLEARALL); // set all styles
    setAStyle(QsciScintillaBase::STYLE_LINENUMBER,(unsigned long)QColor(Qt::white).rgb(),(long)QColor(Qt::darkGray).rgb());
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_SETSELBACK,(unsigned long)1,(long)selection_col.rgb()); // selection color

    setAStyle(SCE_LUA_COMMENT, comment_col, background_col);
    setAStyle(SCE_LUA_COMMENTLINE, comment_col, background_col);
    setAStyle(SCE_LUA_COMMENTDOC, comment_col, background_col);
    setAStyle(SCE_LUA_NUMBER, number_col, background_col);
    setAStyle(SCE_LUA_STRING, string_col, background_col);
    setAStyle(SCE_LUA_LITERALSTRING, string_col, background_col);
    setAStyle(SCE_LUA_CHARACTER, character_col, background_col);
    setAStyle(SCE_LUA_OPERATOR, operator_col, background_col);
    setAStyle(SCE_LUA_PREPROCESSOR, preprocessor_col, background_col);
    setAStyle(SCE_LUA_WORD, keyword1_col, background_col);
    setAStyle(SCE_LUA_WORD2, keyword2_col, background_col);
    setAStyle(SCE_LUA_WORD3, keyword3_col, background_col);
    setAStyle(SCE_LUA_WORD4, keyword4_col, background_col);
    setAStyle(SCE_LUA_IDENTIFIER, identifier_col, background_col);

    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_INDICSETSTYLE,(unsigned long)20,(long)QsciScintillaBase::INDIC_STRAIGHTBOX);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_INDICSETALPHA,(unsigned long)20,(long)160);
    _scintillaObject->SendScintilla(QsciScintillaBase::SCI_INDICSETFORE,(unsigned long)20,(long)selection_col.rgb());
}

void CScintillaDlg::closeEvent(QCloseEvent *event)
{
    if(isModal)
    {
        *modalData.text = _scintillaObject->text();
        modalData.positionAndSize[0] = x();
        modalData.positionAndSize[1] = y();
        modalData.positionAndSize[2] = width();
        modalData.positionAndSize[3] = height();
        modalData.sem->release();
        QDialog::closeEvent(event);
    }
    else
    {
        event->ignore();
        ui->notifyEvent("close", handle);
    }
}

std::string CScintillaDlg::getCallTip(const char* txt)
{
    // e.g.
    if (strcmp(txt,"sim.getObjectHandle")==0)
        return("number handle=sim.getObjectHandle(string objectName)");
    return("");
}

void CScintillaDlg::charAdded(int charAdded)
{
    if (_scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCACTIVE)!=0)
    { // Autocomplete is active
        if (charAdded=='(')
            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCCANCEL);
    }
    if (_scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCACTIVE)==0)
    { // Autocomplete is not active
        if (_scintillaObject->SendScintilla(QsciScintillaBase::SCI_CALLTIPACTIVE)!=0)
        { // CallTip is active
        }
        else
        { // Calltip is not active
            if ( (charAdded=='(')||(charAdded==',') )
            { // Do we need to activate a calltip?

                char linebuf[1000];
                int current=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)999,linebuf);
                int pos=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
                linebuf[current]='\0';
                std::string line(linebuf);
                // 1. Find '('. Not perfect, will also detect e.g. "(" or similar
                int cnt=0;
                int pahr=-1;
                for (pahr=current-1;pahr>0;pahr--)
                {
                    if (line[pahr]==')')
                        cnt--;
                    if (line[pahr]=='(')
                    {
                        cnt++;
                        if (cnt==1)
                            break;
                    }
                }
                if ( (cnt==1)&&(pahr>0) )
                { // 2. Get word
                    int spaceCnt=0;
                    int startword=pahr-1;
                    int endword=startword;
                    while ((startword>=0)&&(isalpha(line[startword])||isdigit(line[startword])||(line[startword]=='_')||(line[startword]=='.')||((line[startword]==' ')&&(spaceCnt>=0)) ))
                    {
                        if (line[startword]==' ')
                        {
                            if ( (spaceCnt==0)&&(endword!=startword) )
                                break;
                            spaceCnt++;
                            endword--;
                        }
                        else
                        {
                            if (spaceCnt>0)
                                spaceCnt=-spaceCnt;
                        }
                        startword--;
                    }
                    std::string s;
                    if (startword!=endword)
                    {
                        s.assign(line.begin()+startword+1,line.begin()+endword+1);
                        s=getCallTip(s.c_str());
                    }
                    if (s!="")
                    {
                        // tabs and window scroll are problematic : pos-=line.size()+startword;
                        setAStyle(QsciScintillaBase::STYLE_CALLTIP,Qt::black,Qt::white,fontSize,theFont);
                        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_CALLTIPUSESTYLE,(int)0);

                        int cursorPosInPixelsFromLeftWindowBorder=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_POINTXFROMPOSITION,(int)0,(unsigned long)pos);
                        int callTipWidthInPixels=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_TEXTWIDTH,QsciScintillaBase::STYLE_CALLTIP,s.c_str());
                        int charWidthInPixels=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_TEXTWIDTH,QsciScintillaBase::STYLE_DEFAULT,"0");
                        int callTipWidthInChars=callTipWidthInPixels/charWidthInPixels;
                        int cursorPosInCharsFromLeftWindowBorder=-5+cursorPosInPixelsFromLeftWindowBorder/charWidthInPixels; // 5 is the width in chars of the left border (line counting)
                        int cursorPosInCharsFromLeftBorder=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCOLUMN,(int)pos);
                        unsigned off=-std::min(cursorPosInCharsFromLeftWindowBorder,cursorPosInCharsFromLeftBorder);
                        if (callTipWidthInChars<std::min(cursorPosInCharsFromLeftWindowBorder,cursorPosInCharsFromLeftBorder))
                            off=-callTipWidthInChars;

                        _scintillaObject->SendScintilla(QsciScintillaBase::SCI_CALLTIPSHOW,(unsigned long)pos+off,s.c_str());
                    }
                }
            }
            else
            { // Do we need to activate autocomplete?
                int p=-1+_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURRENTPOS);
                if (p>=2)
                {
                    char linebuf[1000];
                    int current=_scintillaObject->SendScintilla(QsciScintillaBase::SCI_GETCURLINE,(unsigned long)999,linebuf);
                    linebuf[current]='\0';
                    std::string line(linebuf);
                    int ind=(int)line.size()-1;
                    int cnt=0;
                    std::string theWord;
                    while ((ind>=0)&&(isalpha(line[ind])||isdigit(line[ind])||(line[ind]=='_')||(line[ind]=='.') ))
                    {
                        theWord=line[ind]+theWord;
                        ind--;
                        cnt++;
                    }
                    if (theWord.size()>=3)
                    {
                        // Here we need to create a list with all keywords that match "theWord". e.g.
                        std::string autoCompList="sim.getObjectHandle sim.getObjectName";
                        if (autoCompList.size()!=0)
                        { // We need to activate autocomplete!
                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSETAUTOHIDE,(int)0);
                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSTOPS,(unsigned long)0," ()[]{}:;~`',=*-+/?!@#$%^&|\\<>\"");
//                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSETMAXHEIGHT,(int)100); // it seems that SCI_AUTOCSETMAXHEIGHT and SCI_AUTOCSETMAXWIDTH are not implemented yet!
//                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSETMAXWIDTH,(int)500); // it seems that SCI_AUTOCSETMAXHEIGHT and SCI_AUTOCSETMAXWIDTH are not implemented yet!
                            _scintillaObject->SendScintilla(QsciScintillaBase::SCI_AUTOCSHOW,(unsigned long)cnt,&(autoCompList[0]));
                        }
                    }
                }
            }
        }
    }
}

void CScintillaDlg::modified(int,int,const char*,int,int,int,int,int,int,int)
{

}
