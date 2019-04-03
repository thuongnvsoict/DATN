"""
Created on Mon May 14 08:11:45 2018

@author: Thuong
"""
# Import thu vien dung de thao tao voi file csv
import pandas as pd
import numpy as np
import sys
import os
# Doc bang du lieu trong csv vao bien trainData
if (len(sys.argv) == 2 or len(sys.argv) == 5):
    #trainData: Data has label
    trainData = pd.read_csv(sys.argv[1])
    X = trainData.iloc[:, 1:6].values
    y = trainData.iloc[:, 7].values

    from sklearn.preprocessing import LabelEncoder, OneHotEncoder
    labelencoder_y = LabelEncoder()
    y_encoded = labelencoder_y.fit_transform(y)
    onehotencoder = OneHotEncoder()
    y_encoded_reshaped = y_encoded.reshape(-1, 1)
    y_one_hot_encoded = onehotencoder.fit_transform(y_encoded_reshaped).toarray()
    
    # Splitting the trainData into the Training set and Test set
    from sklearn.model_selection import train_test_split
    X_train, X_test, y_train, y_test = train_test_split(X, y_one_hot_encoded, test_size=0.2, random_state=0)
    
    # Feature Scaling
    from sklearn.preprocessing import StandardScaler
    sc = StandardScaler()
    X_train = sc.fit_transform(X_train)
    X_test = sc.transform(X_test)

    # Part 2 - Now let's make the ANN!
    
    # Importing the Keras libraries and packages
    from keras.models import Sequential
    from keras.layers import Dense
    
    # Initialising the ANN
    classifier = Sequential()
    
    # Adding the input layer and the first hidden layer
    classifier.add(Dense(output_dim = 7, init = 'uniform', activation = 'relu', input_dim = 5))
    
    # Adding the second hidden layer
    classifier.add(Dense(output_dim = 7, init = 'uniform', activation = 'relu'))
    
    # Adding the output layer
    classifier.add(Dense(output_dim = 7, init = 'uniform', activation = 'softmax'))
    
    # Compiling the ANN
    classifier.compile(optimizer = 'adam', loss = 'categorical_crossentropy', metrics = ['accuracy'])
    
    # Fitting the ANN to the Training set
    history = classifier.fit(X_train, y_train, batch_size = len(X_train), nb_epoch = 1200)
    print(history.history.keys())
    # Part 3 - Making the predictions and evaluating the model

    # Predicting the Test set results
    y_pred = classifier.predict(X_test)
    class_pred  = np.argmax(y_pred, axis=1)
    
    class_test = np.argmax(y_test, axis=1)
    
    # Making the Confusion Matrix
    from sklearn.metrics import confusion_matrix
    cm = confusion_matrix(class_pred, class_test) # Error
    print(cm)
else:
    print("Syntax:\npython ann.credit.py <trainData>\nor\nann.credit.py <trainData> <studyData> <outputDir>")
    
if len(sys.argv) == 5:
    #studyData: Data has no label
    studyset_str = pd.read_csv('studyData.csv', dtype=object, index_col=0)
    studyset = pd.read_csv(sys.argv[2])
    #sys.argv[2] = 'studyData.csv'
    X_id = studyset.iloc[:, 0].values
    X_industy = studyset.iloc[:, 6].values
    X_study_raw = studyset.iloc[:, 1:6].values
    X_study = sc.fit_transform(X_study_raw)
    y_study = classifier.predict(X_study)
    class_study = np.argmax(y_study, axis=1)
    rating_list = [None] * len(class_study)
    
    # Convert number to text (Rating)
    for i in range(len(class_study)):
        if class_study[i] == 0:
            rating_list[i] = 'A'
        elif class_study[i] == 1:
            rating_list[i] = 'AA'
        elif class_study[i] == 2:
            rating_list[i] = 'AAA'
        elif class_study[i] == 3:
            rating_list[i] = 'B'
        elif class_study[i] == 4:
            rating_list[i] = 'BB'
        elif class_study[i] == 5:
            rating_list[i] = 'BBB'
        else:
            rating_list[i] = 'CCC'
    
    rating_df = pd.DataFrame(data={'Rating': rating_list})
    output_df = pd.concat([studyset, rating_df], axis=1)
    
    # Print file
    if not os.path.exists(sys.argv[3]):
        os.mkdir(sys.argv[3])
    output_df.to_csv(os.path.join(sys.argv[3], sys.argv[4]), index=False, float_format='%.3f')
