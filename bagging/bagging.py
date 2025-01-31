import numpy as np

class SimplifiedBaggingRegressor:
    def __init__(self, num_bags, oob=False):
        self.num_bags = num_bags
        self.oob = oob
        
    def _generate_splits(self, data: np.ndarray):
        '''
        Generate indices for every bag (bootstrap sampling) and store in self.indices_list.
        Each bag should have the same length as the original data (i.e., len(data)).
        '''
        self.indices_list = []
        data_length = len(data)
        for bag in range(self.num_bags):

            bag_indices = np.random.randint(0, data_length, data_length)
            self.indices_list.append(bag_indices)
        
    def fit(self, model_constructor, data, target):
        '''
        Fit the model on every bag.
        Model constructor with no parameters (and no ()) is passed to this function.

        Example:
            bagging_regressor = SimplifiedBaggingRegressor(num_bags=10, oob=True)
            bagging_regressor.fit(LinearRegression, X, y)
        '''
        self._generate_splits(data)
        
        assert len(set(list(map(len, self.indices_list)))) == 1, \
            'All bags should be of the same length!'
        assert list(map(len, self.indices_list))[0] == len(data), \
            'All bags should contain `len(data)` number of elements!'
        
        self.models_list = []
        for bag in range(self.num_bags):
            model = model_constructor()
            bag_indices = self.indices_list[bag]
            data_bag = data[bag_indices]
            target_bag = target[bag_indices]
            fitted_model = model.fit(data_bag, target_bag)
            self.models_list.append(fitted_model)
        
        if self.oob:
            self.data = data
            self.target = target
        else:
            self.data = None
            self.target = None
        
    def predict(self, data):
        '''
        Get the average prediction for every object from the passed dataset.
        This is the typical bagging ensemble prediction: the mean of predictions
        from all fitted models.
        '''
        predictions = [model.predict(data) for model in self.models_list]
        return np.mean(predictions, axis=0)
    
    def _get_oob_predictions_from_every_model(self):
        '''
        Generates a list of lists, where list i contains predictions for self.data[i]
        from all models that have NOT seen this object during training phase.
        '''
        list_of_predictions_lists = [[] for _ in range(len(self.data))]
        
        for model_idx, bag_indices in enumerate(self.indices_list):
            bag_set = set(bag_indices)
            for i in range(len(self.data)):
                if i not in bag_set:
                  
                    pred = self.models_list[model_idx].predict(self.data[i].reshape(1, -1))
                    list_of_predictions_lists[i].append(pred[0])
        
        self.list_of_predictions_lists = np.array(list_of_predictions_lists, dtype=object)
    
    def _get_averaged_oob_predictions(self):
        '''
        Compute average prediction for every object from the training set based on
        the OOB models only. If an object never appears OOB (i.e., it was in every bag),
        then we assign None for that object.
        '''
        self._get_oob_predictions_from_every_model()
        
        oob_predictions = []
        for preds in self.list_of_predictions_lists:
            if len(preds) == 0:
                oob_predictions.append(None)
            else:
                oob_predictions.append(np.mean(preds))
        self.oob_predictions = np.array(oob_predictions, dtype=object)
        
    def OOB_score(self):
        '''
        Compute mean square error (MSE) for the training objects that have at least one OOB prediction.
        (Skip those that have None as OOB prediction.)
        '''
        self._get_averaged_oob_predictions()
        
        valid_indices = [i for i, val in enumerate(self.oob_predictions) if val is not None]
        if len(valid_indices) == 0:
            return None
        
        valid_oob_preds = np.array([self.oob_predictions[i] for i in valid_indices])
        valid_targets = self.target[valid_indices]
        
        mse = np.mean((valid_oob_preds - valid_targets)**2)
        return mse